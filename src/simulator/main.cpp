#include <algorithm>
#include <bitset>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char* argv[])
{
    double mu = 0.;
    double lambda = 0.;
    std::string activation_mode;
    std::string network_path;
    std::string active_nodes_path;
    double alpha = 0.;
    std::size_t step_count = 0;
    std::string model;
    std::string output_folder;
    std::size_t repetition_count = 1;
    bool keep_intermediate_output = false;
    po::options_description desc("Program options");
    desc.add_options()("help", "Produce help message")(
        "network", po::value<std::string>(&network_path)->required(), "Network path")(
        "activation_mode", po::value<std::string>(&activation_mode)->default_value("all"), "Activation mode: 'all' - activate all nodes, 'file' - read nodes from file, provided by --active-nodes option.")(
        "active_nodes", po::value<std::string>(&active_nodes_path), "Active nodes path")(
        "model", po::value<std::string>(&model)->default_value("A"), "Activity propagation model")(
        "mu", po::value<double>(&mu)->default_value(1.0), "Deactivation rate")(
        "lambda", po::value<double>(&lambda)->required(), "Activity propagation rate")(
        "step_count", po::value<std::size_t>(&step_count)->default_value(10 * 1000 * 1000), "Step count")(
        "output", po::value<std::string>(&output_folder)->default_value("."), "Output folder")(
        "keep_intermediate_output", po::value<bool>(&keep_intermediate_output)->default_value(false), "Keep output for all repetitions. If false, only averaged trajectory will be saved.")(
        "repetitions", po::value<std::size_t>(&repetition_count)->default_value(1), "Repetition count");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (po::error& e) {
        std::cerr << "\nError parsing command line: " << e.what() << std::endl
                  << std::endl;
        std::cerr << desc << std::endl;
        return -1;
    }

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    if (!fs::exists(network_path)) {
        std::cerr << "Invalid network file path." << std::endl;
        return -1;
    }

    if (!fs::exists(output_folder)) {
        fs::create_directory(output_folder);
    }

    std::vector<std::set<std::size_t> > adj;

    std::size_t N = 0;
    std::ifstream network_file(network_path);
    if (network_file.is_open()) {
        network_file >> N;
        adj.resize(N);
        std::size_t v1, v2;
        while (network_file >> v1 >> v2) {
            adj[v1].insert(v2);
            adj[v2].insert(v1);
        }
    }

    boost::dynamic_bitset<> initial_states(N, activation_mode == "file" ? std::numeric_limits<unsigned long>::max() : 0);
    // State array of the nodes. if states[i]==true then node i is inactive else node i is active.

    if (activation_mode == "file") {
        if (!fs::exists(active_nodes_path)) {
            std::cerr << "Invalid active nodes file path." << std::endl;
            return -1;
        }
        std::ifstream active_nodes_file(active_nodes_path);
        if (active_nodes_file.is_open()) {
            std::size_t v;
            while (network_file >> v) {
                if (v < N) {
                    initial_states[v] = false;
                } else {
                    std::cerr << "Invalid vertex index." << std::endl;
                    return -1;
                }
            }
        }
    }

    double propagation_p = lambda / (lambda + mu); // the probability of activity propagation reaction.
    double deactication_p = mu / (lambda + mu); // the probability of the node deactivation reaction.

    std::bernoulli_distribution bernoulli_deactivation(deactication_p);
    std::bernoulli_distribution bernoulli_propagation(propagation_p);

    std::vector<std::string> output_file_names;

    std::random_device rd;

    std::vector<long double> averaged_points(step_count, 0.0);

#pragma omp parallel for
    for (std::size_t r = 0; r < repetition_count; ++r) {
        //std::random_device rd;
        //std::mt19937 gen(rd());
        unsigned int duration = 0;
#pragma omp critical
        {
            duration = rd();
        }
        std::mt19937 gen(duration);

        std::stringstream name;
        std::ofstream fout;
        if (keep_intermediate_output) {
            name << output_folder << "/result_" << lambda << "_" << r << ".txt";
            fout.open(name.str());
        }
        /*if (!fout.is_open()) {
            std::cerr << "Cannot create output file." << std::endl;
            return -1;
        }*/
        if (keep_intermediate_output) {
#pragma omp critical
            {
                output_file_names.emplace_back(name.str());
            }
        }

        std::size_t time = 0;
        std::size_t cache_size = 100000;
        std::vector<long double> points(cache_size);
        boost::dynamic_bitset<> states = initial_states;

        while (time < step_count) {
            if (states.all()) {
                // if all nodes are passive then break simulation.
                break;
            }

            if (0 < time && 0 == time % cache_size) {
                std::cout << "rep = " << r << "; lambda = " << lambda << "; time = " << time << std::endl;
                if (keep_intermediate_output) {
                    for (std::size_t i = 0; i < cache_size; ++i) {
                        fout << time - cache_size + i << " " << points[i] << "\n";
                    }
                }
                std::vector<long double>(cache_size).swap(points);
            }

            std::size_t passive_count = states.count();
            points[time % cache_size] = (N - passive_count) / static_cast<long double>(N);
#pragma omp critical
            {
                averaged_points[time] += points[time % cache_size];
            }

            std::vector<std::size_t> active_nodes;
            for (std::size_t i = 0; i < N; i++) {
                if (!states[i]) {
                    active_nodes.emplace_back(i);
                }
            }
            std::uniform_int_distribution<std::size_t> uid{ 0, active_nodes.size() - 1 };
            std::size_t node = active_nodes[uid(gen)];

            if (1 == bernoulli_deactivation(gen)) {
                // deactivate node
                states[node] = true;
            } else {
                // based upon model, activate one random inactive neighbour or all inactive neighbours and deactivate self
                std::vector<std::size_t> inactive_neighbours;
                for (std::size_t i : adj[node]) {
                    if (states[i]) {
                        inactive_neighbours.emplace_back(i);
                    }
                }

                if (!inactive_neighbours.empty()) {
                    // Model A
                    std::uniform_int_distribution<std::size_t> uid{ 0, inactive_neighbours.size() - 1 };
                    std::size_t node_2 = inactive_neighbours[uid(gen)];
                    states[node_2] = false;
                    // Model B
                    //for (std::size_t y = 0; y < inactive_neighbours.size(); ++y)
                    //{
                    //    if (1 == bernoulli_propagation(gen))
                    //    {
                    //        states[inactive_neighbours[y]] = false;
                    //    }
                    //}
                }
                // Model B
                //states[active_nodes[rand_index]] = true;
            }
            ++time;
        }
        if (keep_intermediate_output) {
            for (size_t i = 0; i < cache_size; ++i) {
                fout << time - cache_size + i << " " << points[i] << "\n";
            }
            fout.close();
        }
    }

    if (repetition_count > 1) {
        std::stringstream final_file_name;
        final_file_name << output_folder << "/result_" << lambda << "_final.txt";
        std::ofstream final_file(final_file_name.str());
        if (!final_file.is_open()) {
            std::cerr << "Cannot create output file." << std::endl;
            return -1;
        }
        for (std::size_t i = 0; i < step_count; ++i) {
            averaged_points[i] /= repetition_count;
            if ((averaged_points[i] - 0.0) < 10e-10) {
                break;
            }
            final_file << i << " " << averaged_points[i] << "\n";
        }
        /*
        // compute average over all repetitions
        std::vector<std::pair<std::size_t, long double>> points;
        std::vector<std::ifstream> files;
        for (std::size_t r = 0; r < repetition_count; ++r) {
            files.emplace_back(std::move(std::ifstream(output_file_names[r])));
        }
        boost::dynamic_bitset<> file_eofs(repetition_count);
        while (!file_eofs.all()) {
            // read single line from all files
            std::size_t step = 0;
            std::vector<long double> values(repetition_count, 0.0L);
            for (std::size_t r = 0; r < repetition_count; ++r) {
                if(files[r] && !files[r].eof()) {
                    files[r] >> step >> values[r];
                } else {
                    file_eofs[r] = true;
                    if(file_eofs.all()) {
                        break;
                    }
                }
            }
            long double result = std::accumulate(values.begin(), values.end(), 0.0L) / repetition_count;
            final_file << step << " " << result << "\n";
        }*/
        final_file.close();
    }

    return 0;
}
