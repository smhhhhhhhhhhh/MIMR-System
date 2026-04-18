#ifndef MIMR_H
#define MIMR_H
 
#include <iostream>
#include <vector>
#include <unordered_map>

struct Topic {
    // Topic attributes
    std::string id;
    std::string name;
    std::string category;
    int mastery;
    int difficulty;
    std::string last_review_date;

    // Topic methods
    int days_since_review() const;
    double get_priority() const;
    bool is_recent(int threshold=14) const;

};

struct AnalysisData {
    double avg_mastery;
    int below_threshold;
};

struct StudyManager {
    // Topic container
    std::vector<Topic> topics;

    // Manager methods
    std::string generate_id() const;

    void add_topic(const std::string& name, const std::string& category, int mastery,
                   int difficulty, const std::string& last_review_date);

    std::vector<Topic> generate_queue(int limit = 10) const;
    bool update_mastery(const std::string& topic_id, int new_mastery);

    std::unordered_map<std::string, std::vector<Topic>> group_by_category() const;
    std::unordered_map<std::string, AnalysisData> get_analysis(int threshold=50) const;

    int count_weak_categories(int threshold=50) const;
    std::vector<std::string> get_weak_categories(int threshold=50) const;
    std::string get_weakest_category() const;

    void save_to_json(const std::string& filename) const;
    void load_from_json(const std::string& filename);
    
};

#endif