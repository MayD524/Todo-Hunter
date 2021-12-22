#include "tabulate.hpp"
#include "argparse.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

using namespace std;
using namespace tabulate;
using Row_t = Table::Row_t;

#ifdef _WIN32
 // clear console
 #define clear() system("cls")
 #define UPDATE_TIME 5
#elif __linux__
 // clear console
 #define clear() system("clear")
 #define UPDATE_TIME 1
#endif

typedef vector<string> string_vector;

struct todo_item {
	string file_path;
	string line_text;
	int line_no;
	int level = 0; // levels increase with the # of 'O's
};

bool isRunning = true;

typedef vector<todo_item> todo_list;

void sleep(const int& seconds) {
	this_thread::sleep_for(chrono::seconds(seconds));
}
// Read function
string_vector read_file(string filename) {
    string_vector lines;
    string line;
    ifstream file(filename);
    if (file.is_open()) {
	while (getline(file, line)) {
	    lines.push_back(line);
	}
	file.close();
    }
    return lines;
}

bool isCommented(const string& line) {
    return line.find("//") != string::npos || line.find("#") != string::npos;
}

todo_list parse_file(const string& file_path, string_vector lines) {
    todo_list todo_items;
    for (int i = 0; i < lines.size(); i++) {
	string line = lines[i];
	if (!isCommented(line)) {
	    continue;
	}
	if ((line.find("todo") != string::npos|| line.find("TODO") != string::npos) && line.find(":") != string::npos) {
	    todo_item item;
	    // get the number of 'O's following the todo
	    int level = -2;
	    bool inComment = false;
	    for (int j = 0; j < line.size(); j++) {
		if (line[j] == '#' || line[j] == '/' && line[j+1] == '/') 
		    inComment = true;
		if (line[j] == 'O'|| line[j] == 'o') {
		    level++;
		}
		if (line[j] == ':')
		    break;
	    }
	    item.line_text = line;
	    item.line_no = i + 1;
	    item.level = level;
	    item.file_path = file_path;
	    todo_items.push_back(item);
	}
    }
    return todo_items;
}

// sort the todo_list by level
todo_list sort_todo_list(todo_list todo_items) {
    sort(todo_items.begin(), todo_items.end(), [](const todo_item& a, const todo_item& b) {
	return a.level > b.level;
    });
    return todo_items;
}
// get a list of all files in a directory
string_vector get_files(const string& dir) {
	string_vector files;
	// getting recursive directory listing was easy LMAO
	for (const auto& entry : filesystem::recursive_directory_iterator(dir)) {
		
		// check if the file is a regular file 
		if (filesystem::is_regular_file(entry.path())) {
			if (entry.path().extension() != ".out" && 
					entry.path().extension() != ".exe" && 
					entry.path().extension() != ".bin" &&
					entry.path().extension() != "") {
				files.push_back(entry.path().string());
			}
		}
		
		//files.push_back(entry.path().string());
	}
	return files;
}

void render_terminal(const string& dir, const string& ext)
{
	string_vector files = get_files(dir);
	for (const auto& file : files) {
		if (ext.compare("*") == 0) 
			goto handle;
		else if (file.find(ext) == string::npos) 
			continue;

		handle:
		string_vector lines = read_file(file);
		todo_list todo_items = parse_file(file, lines);
		todo_items = sort_todo_list(todo_items);
		for (const auto& item : todo_items) {
			cout << item.level << ":" << item.file_path << ":" << item.line_no << ": " << item.line_text << endl;
		}
	}
}
// makes it look pretty
Table render_termTab(const string& dir, const string& ext)
{
	string_vector files = get_files(dir);

	Table prog_table;
	prog_table.add_row({"Level", "Name", "Line #", "Line Data"});
	for (const auto& file : files) {
		if (ext.compare("*") == 0) 
			goto handle;
		else if (file.find(ext) == string::npos) 
			continue;

		handle:
		string_vector lines = read_file(file);
		todo_list todo_items = parse_file(file, lines);
		todo_items = sort_todo_list(todo_items);
		for (const auto& item : todo_items) {
			prog_table.add_row({to_string(item.level), item.file_path, to_string(item.line_no), item.line_text});
		}
	}

	prog_table.column(0).format()
		.font_align(FontAlign::right)
		.font_color(Color::red)
		.font_style({FontStyle::bold});

	prog_table.column(1).format()
		.font_align(FontAlign::right)
		.font_color(Color::blue)
		.font_style({FontStyle::bold});

	prog_table.column(2).format()
		.font_align(FontAlign::left)
		.font_color(Color::cyan)
		.font_style({FontStyle::bold});

	prog_table.column(3).format()
		.font_align(FontAlign::left)
		.font_color(Color::green)
		.font_style({FontStyle::bold});

	for (size_t i = 0; i < 4; i++) {
		prog_table[0][i].format()
			.font_color(Color::yellow)
			.font_align(FontAlign::center)
			.font_style({FontStyle::bold});
	}
	return prog_table;
	//cout << prog_table << endl;
}

int main ( int argc, char *argv[] )
{
	argparse::ArgumentParser program("Todo Hunter");
	program.add_argument("-d", "--directory")
		.help("Directory to search for files")
		.default_value(".");
	program.add_argument("-e", "--extension")
		.help("File extension to search for")
		.default_value("*");
	
	program.parse_args(argc, argv);
	
	string dir = ".";
	try { 
		dir = program.get<string>("directory");
	} catch (const bad_any_cast& e) {
		cout << "No directory specified" << endl;
	}
	

	string ext = "*";
	try { 
		ext = program.get<string>("extension");
	} catch (const bad_any_cast& e){
		cout << "No extension specified" << endl;
	}
	// todoo: find the best time for the sleep
	while (isRunning) {
		// This just makes the writing take place
		// after the tab is rendered and then clears and prints
		Table table = render_termTab(dir, ext);
		clear();
		cout << table << endl;
		//render_terminal(dir, ext);
		sleep(UPDATE_TIME);
	}
}
