#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
	std::string input_file_name;
	double epsilon;
	std::string output_file_name;
	po::options_description desc("Program options");
	desc.add_options()("help", "Produce help message")(
		 "input", po::value<std::string>(&input_file_name)->required(), "Input file name.")(
		 "bin", po::value<double>(&epsilon)->required(), "Bin width.")(
		 "output", po::value<std::string>(&output_file_name)->required(), "Output file name.");

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

	std::ifstream in(input_file_name);
	if (!in.is_open())
	{
		std::cerr << "Cannot open input file." << std::endl;
		return -1;
	}
	std::vector<double> values;
	std::string line;
	while (std::getline(in, line))
	{
		line.erase(line.begin(), std::find_if(line.begin(), line.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		values.push_back(std::stod(line));
	}
	in.close();
	std::vector<int> density(values.size(), 1);
#pragma omp parallel for
	for (int i = 0; i < values.size(); ++i)
	{
		double value = values[i];
		for (int j = 0; j < values.size(); ++j)
		{
			if (j != i && values[j] > value - epsilon && values[j] < value + epsilon)
			{
				++density[i];
			}
		}
	}
	std::ofstream out(output_file_name);
	if (!out.is_open())
	{
		std::cerr << "Invalid output file." << std::endl;
		return -1;
	}
	for (std::size_t i = 0; i < density.size(); ++i)
	{
		out << values[i] << " " << density[i] << std::endl;
	}
	out.close();
	return 0;
}