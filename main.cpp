#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;
  string input;

  while(true){
    cout << "$ ";

    if(input == "\\q"){
      cout << "Выход из shell";
      break;
    }
    if(!input.emty()){
      cout << "Введённая строка " << input << endl;
    }
    if(!getline(cin, input)){
      cout << "Ctrl+D";
      break;
    }
     if(input.find("echo ") == 0){
       cout << input.substr(4) << endl;
    }
    ofstream fout;
    fout.open("kubsh_history.txt");
    fout << input;
    fout.close();
  }
}


