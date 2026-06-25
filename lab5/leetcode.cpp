#include <vector>
#include <iostream>
#include <string>


class Solution final {
private:
    std::vector<int> kmp(const std::string& text, const std::string& pattern) {
        std::vector<std::size_t> shifts(pattern.size(), 0);
        for (std::size_t i = 0; i < pattern.size(); ++i) {
            if (i == 0) {
                shifts[i] = 0;
                continue;
            }

            if (shifts[i - 1] == 0 && pattern[i] == pattern[0]) ++shifts[i];
            else {
                std::size_t pos = shifts[i - 1];
                if (pattern[pos] == pattern[i]) {
                    shifts[i] += shifts[i - 1] + 1;
                }
            }
        }

        std::vector<int> result{};
        for (std::size_t i = 0, j = 0; i < text.size(); ++i) {
            while (j > 0 && text[i] != pattern[j]) {
                j = shifts[j - 1];
            }

            if (j == 0 && text[i] != pattern[j]) {
                j = 0;
                continue;
            }

            ++j;
            if (j == pattern.size()) {
                result.push_back(static_cast<int>(i - j + 1));
                j = shifts[j - 1];
                continue;
            }
        }

        return result;
    }

public:
    std::vector<int> beautifulIndices(std::string s, std::string a, std::string b, int k) {
        std::vector<int> aPos = kmp(s, a);
        std::vector<int> bPos = kmp(s, b);
        
        std::vector<int> result;
        for (size_t i = 0, j = 0; i < aPos.size(); ++i) {
            int curPos = aPos[i];
            
            while (j < bPos.size() && bPos[j] < curPos - k) {
                ++j;
            }
            
            if (j < bPos.size() && bPos[j] <= curPos + k) {
                result.push_back(curPos);
            }
        }

        return result;
    }
};