#include <algorithm>
#include <bitset>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

std::size_t power(std::size_t base, std::size_t exponent)
{
    if (0 == exponent)
    {
        return 1;
    }
    if (0 == base)
    {
        return 0;
    }
    std::size_t result = base;
    while (--exponent != 0)
    {
        result *= base;
    }
    return result;
}

int main(int argc, char *argv[])
{
    std::size_t S = 0;
    std::size_t b = 0;
    std::size_t M0 = 0;
    double p = 0.;
    double alpha = 0.;
    std::string output_file_name;
    po::options_description desc("Program options");
    desc.add_options()("help", "Produce help message")(
        "S", po::value<std::size_t>(&S)->required(), "Level count")(
        "b", po::value<std::size_t>(&b)->required(), "Block size")(
        "M_0", po::value<std::size_t>(&M0)->required(), "Module size")(
        "p", po::value<double>(&p)->default_value(0.25), "Probability")(
        "alpha", po::value<double>(&alpha)->default_value(1.0), "Alpha")(
        "output", po::value<std::string>(&output_file_name)->required(), "Output file name");

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch (po::error &e)
    {
        std::cerr << "\nError parsing command line: " << e.what() << std::endl
                  << std::endl;
        std::cerr << desc << std::endl;
        return false;
    }

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 1;
    }

    const std::size_t N = power(b, S + 1);
    std::map<size_t, std::set<std::size_t>> adj;

    std::random_device rd;
    std::mt19937 gen(rd());

    // step 1: generation of the fully connected blocks
    // N/M0 the count of blocks at the 0 level
    for (std::size_t i = 0; i < N / M0; i++)
    {
        for (std::size_t j = i * M0; j < (i + 1) * M0; j++)
        {
            for (std::size_t k = i * M0; k < (i + 1) * M0; k++)
            {
                if (k != j)
                {
                    adj[j].insert(k);
                    adj[k].insert(j);
                }
            }
        }
    }

    for (std::size_t l = 1; l <= S; ++l)
    {
        // at level l
        double p_l = alpha * std::pow(p, l); // TODO: check for l+1
        std::cout << "p_l = " << p_l << std::endl;
        std::size_t block_count = N / power(b, l);
        std::cout << "block_count = " << block_count << std::endl;
        std::size_t block_size = N / block_count;
        std::cout << "block_size = " << block_size << std::endl;

        std::bernoulli_distribution bernoulli_d(p_l);

        for (std::size_t current_b = 0; current_b < block_count; current_b += b)
        {
            std::cout << "current block " << current_b << " from " << block_count << std::endl;
            for (std::size_t block1 = current_b; block1 < current_b + b; ++block1)
            {
                //std::cout << "block1 = " << block1 << std::endl;
                for (std::size_t block2 = block1 + 1; block2 < current_b + b; ++block2)
                {
                    //std::cout << "block2 = " << block2 << std::endl;
                    bool added_edge = false;
                    //#pragma omp parallel for
                    for (std::size_t i = 0; i < block_size; ++i)
                    {
                        std::size_t index1 = block1 * block_size + i;
                        //#pragma omp parallel for
                        for (std::size_t j = 0; j < block_size; ++j)
                        {
                            std::size_t index2 = block2 * block_size + j;
                            if (index1 != index2 && 1 == bernoulli_d(gen))
                            {
                                adj[index1].insert(index2);
                                adj[index2].insert(index1);
                                added_edge = true;
                            }
                        }
                    }
                    if (!added_edge)
                    {
                        --block2;
                    }
                }
            }
        }
    }

    std::ofstream network_file(output_file_name);
    if (network_file.is_open())
    {
        network_file << N << std::endl;
        std::set<std::pair<std::size_t, std::size_t>> checker;
        for (auto &row : adj)
        {
            for (auto &col : row.second)
            {
                if (checker
                        .insert(std::make_pair(std::min(row.first, col),
                                               std::max(row.first, col)))
                        .second)
                {
                    network_file << row.first << " " << col << "\n";
                }
            }
        }
        network_file.close();
    }
    return 0;
}
