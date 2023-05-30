#include "./WordIndex.h"
#include <bits/stdc++.h>
#include <unordered_set>

namespace searchserver {

    WordIndex::WordIndex() {
        // TODO: implement

    }

    size_t WordIndex::num_words() {
        // TODO: implement
        return m.size();
    }

    void WordIndex::record(const string &word, const string &doc_name) {
        // TODO: implement
        if (m.find(word) == m.end()) {
            m[word][doc_name] = 1;
        } else {
            if (m[word].find(doc_name) == m[word].end()) {
                m[word][doc_name] = 1;
            } else {
                m[word][doc_name]++;
            }
        }
    }

    bool compareRank(Result r1, Result r2) {
        return r2.rank < r1.rank;
    }

    vector<Result> WordIndex::lookup_word(const string &word) {
        vector<Result> result;
        // TODO: implement

        auto doc_rank = m[word];
        for (auto &item: doc_rank) {
            result.push_back(Result(item.first, item.second));
        }

        std::sort(result.begin(), result.end(), compareRank);

        return result;
    }

    vector<Result> WordIndex::lookup_query(const vector<string> &query) {
        vector<Result> results;

        // TODO: implement
        if (query.size() == 0) {
            return results;
        }

        std::unordered_map<string, int> res_map;
        std::unordered_set<string> doc_set;

        for (auto &item: m) {
            auto &doc_rank = item.second;
            for (auto &it: doc_rank) {
                doc_set.insert(it.first);
            }
        }

        for (auto &q: query) {
            if (m.find(q) != m.end()) {
                auto &doc_rank = m[q];
                std::unordered_set<string> tmp;
                for (auto &item: doc_rank) {
                    tmp.insert(item.first);
                }
                std::unordered_set<string> del;
                for (auto &it: doc_set) {
                    if (tmp.find(it) == tmp.end()) {
                        del.insert(it);
                    }
                }
                // delete
                for (auto &it: del) {
                    doc_set.erase(it);
                }
            } else {
                doc_set.clear();
                break;
            }
        }
        if (doc_set.size() == 0) {
            return results;
        }

        for (string q: query) {
            if (m.find(q) != m.end()) {
                std::unordered_map<string, int> doc_rank = m[q];
                for (auto &item: doc_rank) {
                    if (doc_set.find(item.first) == doc_set.end()) {
                        continue;
                    }
                    if (res_map.find(item.first) == res_map.end()) {
                        res_map[item.first] = item.second;
                    } else {
                        res_map[item.first] += item.second;
                    }
                }
            }
        }

        for (auto &item: res_map) {
            results.push_back(Result(item.first, item.second));
        }

        std::sort(results.begin(), results.end(), compareRank);


        return results;
    }

}  // namespace searchserver
