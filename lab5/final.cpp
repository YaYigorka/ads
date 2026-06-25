#include <iostream>
#include <map>
#include <algorithm>
#include <string>
#include <queue>
#include <cstddef>

struct Node final {
    std::ptrdiff_t start;
    std::ptrdiff_t end;

    std::map<char, Node*> children;

    std::ptrdiff_t suffix_index;

    Node(std::ptrdiff_t start_, std::ptrdiff_t end_)
        : start{start_}
        , end{end_}
        , children{}
        , suffix_index{-1}
    {
    }

    Node(std::ptrdiff_t start_, std::ptrdiff_t end_, std::ptrdiff_t leaf_index)
        : start{start_}
        , end{end_}
        , children{}
        , suffix_index{leaf_index}
    {
    }

    ~Node() {
        for (auto& child : children) {
            delete child.second;
        }
    }
};

class SuffixTree final {
private:
    std::string& str;
    Node* root;

    void insertSuffix(std::size_t suffix_start) {
        Node* node = root;
        std::size_t str_pos = suffix_start;
        while (str_pos < str.size()) {
            auto it = node->children.find(str[str_pos]);
            if (it == node->children.end()) {
                node->children[str[str_pos]] = new Node{str_pos, str.size(), suffix_start};
                return;
            }

            Node* child = it->second;
            std::ptrdiff_t curr_pos{child->start}, end_pos{child->end};
            while (curr_pos < end_pos && str_pos < str.size()) {
                if (str[str_pos] != str[curr_pos]) {
                    Node* middle = new Node{child->start, curr_pos};
                    node->children[str[child->start]] = middle;
                    middle->children[str[str_pos]] = new Node{str_pos, str.size(), suffix_start};
                    child->start = curr_pos;
                    middle->children[str[child->start]] = child;

                    return;
                }

                ++curr_pos;
                ++str_pos;

            }

            node = child;
        }

        return;
    }

    const std::vector<std::size_t> collectLeafs(Node* node) const {
        std::vector<std::size_t> result{};

        std::queue<Node*> queue{};
        queue.push(node);
        while (!queue.empty()) {
            Node* current = queue.front();
            queue.pop();

            if (current->suffix_index != -1) {
                result.push_back(current->suffix_index + 1);
            }

            for (const auto& child : current->children) {
                queue.push(child.second);
            }
        }
        std::sort(result.begin(), result.end());
        return result;
    }

public:
    ~SuffixTree() { delete root; }
    SuffixTree(std::string& str)
        : str{str}, root{new Node{-1, -1}}
    {
    }

    void buildTree() {
        for (std::size_t i = 0; i < str.size(); ++i) {
            insertSuffix(i);
        }
    }

    std::vector<std::size_t> searchPattern(const std::string& pattern) const {
        Node* node = root;
        std::size_t pos = 0;
        while (pos < pattern.size()) {
            auto it = node->children.find(pattern[pos]);
            if (it == node->children.end()) return {};

            Node* child = it->second;
            std::ptrdiff_t curr_pos{child->start}, end_pos{child->end};
            while (curr_pos < end_pos && pos < pattern.size()) {
                if (str[curr_pos] != pattern[pos]) return {};
                
                ++curr_pos;
                ++pos;
            }

            node = child;
        }

        return collectLeafs(node);
    }
};


int main() {
 std::string inputLine;
    std::getline(std::cin, inputLine);
    inputLine += '$';

    SuffixTree suffixTree{inputLine};
    suffixTree.buildTree();

    std::string patternLine;
    int patternCounter = 1;
    while (std::getline(std::cin, patternLine)) {
        if (!patternLine.empty()) {
            auto positions = suffixTree.searchPattern(patternLine);
            if (!positions.empty()) {
                std::cout << patternCounter << ": ";
                const int posCount = static_cast<int>(positions.size());
                for (int k = 0; k < posCount; ++k) {
                    if (k) std::cout << ", ";
                    std::cout << positions[k];
                }
                
                std::cout << "\n";
            }
        }

        ++patternCounter;
    }

    return 0;
}