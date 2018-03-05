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

	std::size_t n = values.size();

	// first calculate mean value and if it's != 0, subtract from input data
	long double mean = .0;
	for(const long double& val : values)
	{
		mean += val;
	}
	mean /= n;
	if(mean > std::numeric_limits<long double>::epsilon())
	{
		for(long double& val : values)
		{
			val -= mean;
		}
	}

	// calcute dispersion of input data
	long double dispersion = .0;
	for(const long double& val : values)
	{
		dispersion += val*val;
	}
	dispersion /= n;

	std::vector<long double> autocorrelation_f(n, .0);

	// for each fixed value of time period calculate auto-correlation
	// maximum value for time period is the length of input data
	for(std::size_t t = 0; t < n; ++t)
	{
		// now consider all pairs of input data having distance equal to t
		// we need a window of size t, which we'll slide through our data and collect mentioned pairs
		long double autocorrelation = .0;
		long double factor = 1.0 / ((n-t)*dispersion);
		for(std::size_t i = 0; i < n - t; ++i)
		{
			autocorrelation += values[i]*values[i+t];
		}
		autocorrelation *= factor;
		autocorrelation_f[t] = autocorrelation;
	}

	std::ofstream out(output_file_name);
	if (!out.is_open())
	{
		std::cerr << "Invalid output file." << std::endl;
		return -1;
	}
	for(std::size_t i = 0; i < n; ++i)
	{
		out << i << " " << autocorrelation_f[i] << '\n';
	}
	out.close();
	return 0;
}