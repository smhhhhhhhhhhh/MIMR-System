#include "mimr.h"
#include <iostream>
#include <string>
#include <limits>
#include <vector>
#include <unordered_map>
#include <iomanip>

using std::cout;
using std::cin;
using std::string;
using std::getline;
using std::vector;
using std::unordered_map;

void clean_input() {
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    StudyManager manager;
    string filename = "well.json";
    manager.load_from_json(filename);
    char choice = '0';

    while(true) {
        cout << "\n---MIMR System---\n"
             << "Main Menu\n"
             << "1. Add Topic\n"
             << "2. Review Queue\n"
             << "3. Analytics\n"
             << "4. Save and Exit\n\n"
             << "Pick an option (1-4): ";
        cin >> choice;
        clean_input();        

        switch(choice) {
            case '1': {
                string name, category;
                int mastery=0, difficulty=0;
                cout << "\n---Add Topic---\n";
                cout << "Topic Name (e.g. Linked lists, Stacks): ";
                getline(cin, name);

                cout << "Category (e.g. Data Structures, Calculus): ";
                getline(cin, category);

                while(true) {
                    cout << "Mastery (1-100): ";
                    cin >> mastery;
                    if(cin.fail() || mastery < 1 || mastery > 100) {
                        cout << "\nInvalid input. Try again.\n";
                        cin.clear();
                        clean_input();
                        continue;
                    }
                    break;
                }

                while(true) {
                    cout << "Difficulty (1-5): ";
                    cin >> difficulty;
                    if(cin.fail() || difficulty < 1 || difficulty > 5) {
                        cout << "\nInvalid input. Try again.\n";
                        cin.clear();
                        clean_input();
                        continue;
                    }
                    break;
                }
                manager.add_topic(name, category, mastery, difficulty, manager.get_date_today());
                break;
            }

            case '2': {
                vector<Topic> queue = manager.generate_queue();                
                if(queue.empty()) {
                    cout << "\nNo topics to review.\n";
                    break;
                }

                cout << "\n---Review Queue---\n";
                cout << std::fixed << std::setprecision(2);
                for(size_t i = 0; i < queue.size(); i++) {
                    cout << i+1 << ". "
                         << queue[i].name
                         << " | Mastery: " << queue[i].mastery
                         << " | Difficulty: " << queue[i].difficulty
                         << " | Priority: " << queue[i].get_priority()
                         << "\n";
                }

                int index;
                while(true) {
                    cout << "\nSelect topic to update mastery (0 to exit): ";
                    cin >> index;
                    if(index == 0) break;
                    if(cin.fail() || index < 1 || index > queue.size()) {
                        cin.clear();
                        clean_input();
                        cout << "Invalid input.\n";
                        continue;
                    }
                    break;
                }

                if(index == 0) break;
                int new_mastery;
                while(true) {
                    cout << "\nNew mastery (1-100): ";
                    cin >> new_mastery;
                    if(cin.fail() || new_mastery < 1 || new_mastery > 100) {
                        cin.clear();
                        clean_input();
                        cout << "Invalid input.\n";
                        continue;
                    }
                    break;
                }
                manager.update_mastery(queue[index-1].id, new_mastery);
                break;
            }

            case '3': {
                unordered_map<string, AnalysisData> analysis = manager.get_analysis();
                vector<string> weak_categories = manager.get_weak_categories();
                string weakest = manager.get_weakest_category();

                if(analysis.empty()) {
                    cout << "\nNo data available.\n";
                    break;
                }
                
                cout << "\n--Analytics--\n";
                for(const auto& [category, data] : analysis) {
                    cout << category << " | Average: " << data.avg_mastery 
                         << " | Topics below mastery threshold: " << data.below_threshold
                         << "\n";
                }

                cout << "\n~~~Weak Categories~~~\n";
                int count = manager.count_weak_categories();
                if(!weak_categories.empty()) {
                    for(const string& category : weak_categories) 
                    cout << "- " << category << "\n";
                }
                else cout << "None, keep it up!\n";

                cout << "~~~Weakest Category~~~\n";
                if(!weakest.empty())
                    cout << weakest << "| Average: " << analysis[weakest].avg_mastery << "\n";
                else
                    cout << "No data available.\n";
                break;
            }
            
            case '4':
                manager.save_to_json(filename);
                cout << "\nSaved. Exiting...\n";
                return 0;
        }

    }
}