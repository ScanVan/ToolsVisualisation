//============================================================================
// Name        : Equirectangular-CPP.cpp
// Author      : Marcelo Kaihara
// Version     :
// Copyright   : 
// Description : Code to convert images into the equirectangular representation based on the calibration results.
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <algorithm>
#include <set>
#include <experimental/filesystem>
#include <math.h>
// Include files to use OpenCV API
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace fs = std::experimental::filesystem;

class PathClass {
public:
	std::string ConfigFilePath {"./"};
	std::string path_to_equi {"./"};
	std::string path_to_model {"./"};
	std::string file_2_matches {"./"};
	PathClass(std::string s): ConfigFilePath(s) {};
	PathClass(std::string path_to_equi, std::string path_to_model):
		path_to_equi(path_to_equi), path_to_model(path_to_model) {}
	PathClass(std::string path_to_equi, std::string path_to_model, std::string file_2_matches):
			path_to_equi(path_to_equi), path_to_model(path_to_model), file_2_matches(file_2_matches) {}
	void SetFile2Matches (std::string file) {
		file_2_matches = file;
	}


};


std::string trim(const std::string& str, const std::string& whitespace = " \t") {
// trim white spaces and tabs from the beginning and end

	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

std::string ToUpper(const std::string &str) {
// Convert to upper case the string

	std::locale loc { };
	std::stringstream ss { };
	for (std::string::size_type i = 0; i < str.length(); ++i)
		ss << std::toupper(str[i], loc);
	return (ss.str());
}

void ProcessConfigFile(PathClass &FC) {

	// open configuration file
	std::ifstream f(FC.ConfigFilePath);

	if (!f.good()) {
		throw std::runtime_error("Error opening the configuration file \"" + FC.ConfigFilePath + "\".");
	}

	// process configuration file
	while (f.good()) {

		std::string line { };
		std::getline(f, line);

		// remove white spaces from the line
		std::string trimmedLine = trim(line);
		if (trimmedLine == "")
			continue;

		// Ignore lines starting with hash symbol
		const auto strHash = trimmedLine.find_first_of("#");
		if (strHash == 0)
			continue;

		// Extract the command and the argument separated by the symbol "="
		const auto strEqual = trimmedLine.find_first_of("=");
		std::string command = ToUpper(trim(trimmedLine.substr(0, strEqual)));
		std::string argument = trim(trimmedLine.substr(strEqual + 1));

		// Check for commands
		if (command == "PATH_TO_EQUI") {
			// Path of the equirectangular images
			if (argument != "") {
				fs::path p = argument;
				if (!fs::exists(p)) {
					throw(std::runtime_error("Error: the path " + argument + " does not exist."));
				}
				FC.path_to_equi = argument;
			} else {
				throw(std::runtime_error("Error: PATH_TO_EQUI parameter is empty."));
			}
		} else if (command == "PATH_TO_MODEL") {
			// Path to the models
			if (argument != "") {
				fs::path p = argument;
				if (!fs::exists(p)) {
					throw(std::runtime_error("Error: the path " + argument + " does not exist."));
				}
				FC.path_to_model = argument;
			} else {
				throw(std::runtime_error("Error: PATH_TO_MODEL parameter is empty."));
			}
		} else if (command == "FILE_MATCHES2") {
			// File name of the matches to analyze
			if (argument != "") {
				FC.file_2_matches = argument;
			} else {
				throw(std::runtime_error("Error: FILE_MATCHES2 parameter is empty."));
			}
		}
	}
}

void LoadMatches2 (PathClass &PC, std::vector<std::vector<double>> &P1, std::vector<std::vector<double>> &P2) {

	fs::path path2Models = PC.path_to_model;
	fs::path path2Matches = ((path2Models / std::string("output")) / std::string("2_matches")) / PC.file_2_matches;

	std::ifstream f (path2Matches.string());

	if (!f.good()) {
		throw std::runtime_error("Could not open the file with the matches.");
	}

	P1.clear();
	P2.clear();

	while (f.good()) {
		std::string s {};
		double x1{}, y1{}, x2{}, y2{};

		std::getline(f, s);
		if (s != "") {
			std::stringstream ss { };

			ss << s;

			ss >> std::setprecision(15) >> x1 >> y1 >> x2 >> y2;

			std::vector<double> point1 {};
			point1.push_back(x1);
			point1.push_back(y1);

			P1.push_back(point1);

			std::vector<double> point2 {};
			point2.push_back(x2);
			point2.push_back(y2);

			P2.push_back(point2);

			//std::cout << std::setprecision(15) << x1 << " " << y1 << " " << x2 << " " << y2 << std::endl;
		}

	}

}

double CalcMeanDistance (std::vector<std::vector<double>> &P1, std::vector<std::vector<double>> &P2) {

	if (P1.size()!= P2.size()) {
		throw std::runtime_error("Number of features in image 1 is different to the number of features in image 2");
	}

	double accum {0};

	for (size_t i{0}; i <P1.size(); ++i) {

		accum += sqrt(pow(P1[i][0] - P2[i][0], 2) + pow(P1[i][1] - P2[i][1], 2));

	}

	accum /= P1.size();

	return accum;
}


void ShowMatches2 (PathClass &PC) {

	cv::RNG rng(12345);

	// File name of the matches of pair of images
	std::string filePair = PC.file_2_matches;

	// Extract name of the 2 images separated by "_"
	const auto posUnderscore = filePair.find_first_of("_");
	std::string file1 = filePair.substr(0, posUnderscore) + ".bmp";
	std::string file2 = filePair.substr(posUnderscore + 1) + ".bmp";

	cv::Mat img1 {};
	cv::Mat img2 {};

	// Generate the full path to the image 1
	fs::path pathEqui = PC.path_to_equi;
	fs::path pathEqui1 = pathEqui / file1;
	std::string path_to_equi_1 = pathEqui1.string();

	img1 = cv::imread(path_to_equi_1);
	if (!img1.data) {
		throw std::runtime_error("Could not load the equirectangular image.");
	}

	// Generate the full path to the image 2
	fs::path pathEqui2 = pathEqui / file2;
	std::string path_to_equi_2 = pathEqui2.string();

	img2 = cv::imread(path_to_equi_2);
	if (!img2.data) {
		throw std::runtime_error("Could not load the equirectangular image.");
	}

	std::vector<std::vector<double>> P1;
	std::vector<std::vector<double>> P2;

	LoadMatches2(PC, P1, P2);

	std::cout << "The mean distance of the features from image 1 and image 2 is: " << CalcMeanDistance(P1, P2) << std::endl;

	cv::Mat gray1 { };
	cv::cvtColor(img1, gray1, cv::COLOR_BGR2GRAY);
	cv::Mat bgr[3] { };
	cv::split(img2, bgr);
	std::vector<cv::Mat> images(3);

	for (const auto &point1:P1) {
		cv::circle (img1, cv::Point(point1[0],point1[1]), 5.0, cv::Scalar(0,255,0),1,8);
	}


	for (const auto &point2:P2) {
		cv::circle (img2, cv::Point(point2[0],point2[1]), 5.0, cv::Scalar(0,255,0),1,8);
	}


	images.at(0) = bgr[0];
	images.at(1) = gray1;
	images.at(2) = bgr[2];


	cv::Mat img3 {};
	cv::merge(images, img3);

	for (const auto &point1 : P1) {
		cv::circle(img3, cv::Point(point1[0], point1[1]), 5.0, cv::Scalar(0, 255, 255), 1, 8);
	}

	for (const auto &point2 : P2) {
		cv::circle(img3, cv::Point(point2[0], point2[1]), 5.0, cv::Scalar(0, 255, 255), 1, 8);
	}

	for (size_t i{0}; i < P1.size(); ++i) {
		cv::Scalar lineColor (rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		cv::line(img3, cv::Point(P1[i][0], P1[i][1]), cv::Point(P2[i][0], P2[i][1]), lineColor, 2, cv::LINE_AA);
	}


	cv::namedWindow("Image 3",  cv::WINDOW_NORMAL);
	cv::imshow("Image 3", img3);


	cv::namedWindow("Image 1",  cv::WINDOW_NORMAL);
	cv::imshow("Image 1", img1);

	cv::namedWindow("Image 2", cv::WINDOW_NORMAL);
	cv::imshow("Image 2", img2);

	cv::waitKey(0);


}

int main(int argc, char* argv[]) {

		if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " config_file.txt" << std::endl;
		return 1;
	}

	std::string cfg = argv[1];

	// Create object Path Class
	PathClass PC{cfg};

	// Process the configuration file
	ProcessConfigFile (PC);

	ShowMatches2 (PC);

	return 0;
}
