#include "mimr.h"
#include "json.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <ctime>
#include <iomanip>

using std::cout;
using std::cin;
using std::string;
using std::vector;
using std::unordered_map;
using std::max;
using json = nlohmann::json;

// TOPIC METHODS
// Get the value of days elapsed since last review
int Topic::days_since_review() const {
    std::tm tm = {};
    std::istringstream ss(last_review_date);
    ss >> std::get_time(&tm, "%Y-%m-%d");

    if (ss.fail()) return 0;

    std::time_t review_time = std::mktime(&tm);
    std::time_t now = std::time(nullptr);

    double seconds = std::difftime(now, review_time);
    return static_cast<int>(seconds / (60 * 60 * 24));
}

// Get the priority value of a topic
double Topic::get_priority() const {
    int days_elapsed = std::max(days_since_review(), 1);
    double priority = (static_cast<double>(difficulty) * days_elapsed / (mastery + 1));
    return priority;
}

// Identify whether a topic is considered "finished" or no longer needed
bool Topic::is_recent(int threshold) const {
    return days_since_review() <= threshold;
}

// MANAGER METHODS
// Naming convention
string StudyManager::generate_id() const {
    int next_number = 0;
    for(const auto& t: topics) {
        size_t pos = t.id.find('T');
        if(pos != string::npos) {
            next_number = std::max(next_number, std::stoi(t.id.substr(pos + 1)));
        }
    }
    std::ostringstream oss;
    oss << "MIMR-T" << std::setw(3) << std::setfill('0') << (next_number + 1);
    return oss.str();
}

// Topic addition
void StudyManager::add_topic(const std::string& name, const std::string& category, int mastery,
               int difficulty, const std::string& last_review_date) {
    string new_id = generate_id();
    Topic topic = {new_id, name, category, mastery, difficulty, last_review_date};
    topics.push_back(topic);         
}   

// Get the date today in Y-m-d format
string StudyManager::get_date_today() const {
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);

    char buffer[11];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm);

    return string(buffer);
}

// Review queue generation
vector<Topic> StudyManager::generate_queue(int limit) const {
    vector<Topic> eligible_topics;
    for(const Topic& t : topics) {
        if(t.is_recent(60))
        eligible_topics.push_back(t);
    }
    
    vector<Topic> sorted_topics = eligible_topics;
    std::sort(sorted_topics.begin(), sorted_topics.end(),
              [](const Topic&a, const Topic& b) {
                return a.get_priority() > b.get_priority();
              }
    );

    int end = std::min(limit, (int)sorted_topics.size());
    return vector<Topic>(sorted_topics.begin(), sorted_topics.begin() + end);
}

// Mastery update option
bool StudyManager::update_mastery(const std::string& topic_id, int new_mastery) {
    for(Topic& topic : topics) {
        if(topic.id == topic_id) {
            topic.mastery = new_mastery;
            topic.last_review_date = topic.last_review_date = get_date_today();
            return true;
        }
    }
    return false;
}

// Group topics by category
unordered_map<string, vector<Topic>> StudyManager::group_by_category() const {
    unordered_map<string, vector<Topic>> groups;
    for(const Topic& t : topics) {
        groups[t.category].push_back(t);
    }
    return groups;
}

// Analytics logic
unordered_map<string, AnalysisData> StudyManager::get_analysis(int threshold) const {
    unordered_map<string, vector<Topic>> groups = group_by_category();
    unordered_map<string, AnalysisData> result;

    for(const auto& [category, topics] : groups) {
        double total = 0.0;
        for(const Topic& t : topics)
            total += t.mastery;
        double avg = std::round((total / topics.size()) * 100) / 100;

        int below = 0;
        for(const Topic& t : topics) {
            if(t.mastery < threshold) below++;
        }
        result[category] = AnalysisData{avg, below};
    }
    return result;
}

// Count categories below threshold
int StudyManager::count_weak_categories(int threshold) const {
    unordered_map<string, AnalysisData> analysis = get_analysis(threshold);
    if(analysis.empty()) return 0;
    
    int count = 0;
    for(const auto& [category, data] : analysis) {
        if(data.avg_mastery < threshold) count++;
    }

    return count;
}

// Get the categories below threshold
vector<string> StudyManager::get_weak_categories(int threshold) const {
    auto analysis = get_analysis(threshold);
    if(analysis.empty()) return {};

    vector<string> weak_categories;
    for(const auto& [category, data] : analysis) {
                if(data.avg_mastery < threshold)
                    weak_categories.push_back(category);
    }
    return weak_categories;
}

// Get the weakest category
string StudyManager::get_weakest_category() const {
    unordered_map<string, AnalysisData> analysis = get_analysis();
    if(analysis.empty()) return "";

    // This is a pair data type
    auto weakest = std::min_element(analysis.begin(), analysis.end(),
                   [](const auto& a, const auto& b) {
                    return a.second.avg_mastery < b.second.avg_mastery;
                   });
    return weakest->first;
}

// Save data to json
void StudyManager::save_to_json(const std::string& filename) const {
    json j;
    for(const Topic& topic: topics) {
        j[topic.id] = {
            {"name", topic.name},
            {"category", topic.category},
            {"mastery", topic.mastery},
            {"difficulty", topic.difficulty},
            {"last_review_date", topic.last_review_date}
        };
    }

    std::ofstream file(filename);
    file << j.dump(4);
}

// Load json at start
void StudyManager::load_from_json(const std::string& filename) {
    topics.clear();

    std::ifstream file(filename);
    if(!file.is_open()) return;

    json j = json::parse(file);
    for(const auto& [id, value] : j.items()) {
        Topic topic;
        topic.id = id;
        topic.name = value.value("name", "");
        topic.category = value.value("category", "");
        topic.mastery = value.value("mastery", 0);
        topic.difficulty = value.value("difficulty", 0);
        topic.last_review_date = value.value("last_review_date", "");
        topics.push_back(topic);
    }
}
    