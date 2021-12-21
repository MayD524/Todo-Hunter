#include "argparse.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

#ifdef _WIN32
 // clear console
 #define clear() system("cls")
#elif __linux__
 // clear console
 #define clear() system("clear")
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
// TODO: test
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

// TODOOOOOOO: TEST
bool isCommented(const string& line) {
    return line.find("//") != string::npos || line.find("#") != string::npos;
}
// TODO: test2
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
	    item.line_no = i;
	    item.level = level;
	    item.file_path = file_path;
	    todo_items.push_back(item);
	}
    }
    return todo_items;
}

// sort the todo_list by level
void sort_todo_list(todo_list& todo_items) {
    sort(todo_items.begin(), todo_items.end(), [](const todo_item& a, const todo_item& b) {
	return a.level > b.level;
    });
}

// get a list of all files in a directory
string_vector get_files(const string& dir) {
	string_vector files;
	for (const auto& entry : filesystem::directory_iterator(dir)) {
		files.push_back(entry.path().string());
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
		sort_todo_list(todo_items);
		for (const auto& item : todo_items) {
			cout << item.level << ":" << item.file_path << ":" << item.line_no << ": " << item.line_text << endl;
		}
	}
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
	while (isRunning) {
		clear();
		render_terminal(dir, ext);
		sleep(1);
	}
}
