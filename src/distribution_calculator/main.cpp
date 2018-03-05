#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <regex>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

std::vector<std::string> split(const std::string& input, const std::string& regex)
{
	// passing -1 as the submatch index parameter performs splitting
	std::regex re(regex);
	std::sregex_token_iterator
		 first{input.begin(), input.end(), re, -1},
		 last;
	return {first, last};
}

int main(int argc, char **argv)
{
	std::string input_file_name;
	long double epsilon;
	std::string output_file_name;
	std::size_t column;
	std::size_t start_row;
	po::options_description desc("Program options");
	desc.add_options()("help", "Produce help message")(
		"input", po::value<std::string>(&input_file_name)->required(), "Input file name.")(
		"column", po::value<std::size_t>(&column)->default_value(1), "Column number in input file (1-based).")(
		"start_row", po::value<std::size_t>(&start_row)->default_value(1), "Start row in input file (1-based).")(
		"bin", po::value<long double>(&epsilon)->required(), "Bin width.")(
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
	std::vector<long double> values;
	std::string line;
	std::size_t lines_read = 0;
	while (std::getline(in, line))
	{
		const std::vector<std::string>& tokens = split(line, "\\s");
		if(tokens.size() >= column)
		{
			if(++lines_read > start_row)
			{
				long double val = std::stold(tokens[column-1]);
				values.push_back(val);
			}
		}
		else
		{
			std::cerr << "Invalid input data: one of the lines does not contain enough columns." << std::endl;
			in.close();
			return -1;
		}
	}
	in.close();
	if(values.empty())
	{
		return -1;
	}
	std::vector<std::size_t> density;
	std::sort(values.begin(), values.end());
	std::size_t k = 0;
	long double current_start = values[0];
	for (std::size_t j = 0; j < values.size();)
	{
		density.emplace_back(0);
		while(j < values.size() && values[j++] < current_start + epsilon)
		{
			++density[k];
		}
		k++;
		current_start += epsilon;
	}
	std::ofstream out(output_file_name);
	if (!out.is_open())
	{
		std::cerr << "Invalid output file." << std::endl;
		return -1;
	}
	for (std::size_t i = 0; i < density.size(); ++i)
	{
		out << i*epsilon + epsilon/2 << " " << density[i] << std::endl;
	}
	out.close();
	return 0;
}