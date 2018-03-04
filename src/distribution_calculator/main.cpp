#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		std::cerr << "Invalid usage." << std::endl;
		return -1;
	}
	std::ifstream in(argv[1]);
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
	double epsilon = std::stod(argv[2]);
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
	std::ofstream out(argv[3]);
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