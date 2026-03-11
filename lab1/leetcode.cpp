#include <vector>
#include <cstddef>
#include <cmath>
#include <iostream>


class Solution {
private:
    static void counting_sort(std::vector<int>& vec) {
        std::vector<int> buf(vec.size());

        int k;
        int max_k = 0;
        for (std::size_t i = 0; i != vec.size(); ++i) {
            k = vec[i];
            if (k > max_k) {
                max_k = k;
            }
        }

        std::vector<int> counter(max_k + 1);
        for (std::size_t i = 0; i != vec.size(); ++i) {
            ++counter[vec[i]];
        }

        for (std::size_t i = 1; i <= max_k; ++i) {
            counter[i] += counter[i - 1];
        }

        for (std::size_t i = vec.size(); i > 0; --i) {
            int key = vec[i - 1];
            buf[counter[key] - 1] = key;
            --counter[key];
        }

        vec = std::move(buf);
    }
public:
    static int minMovesToSeat(std::vector<int>& seats, std::vector<int>& students) {
        counting_sort(seats);
        counting_sort(students);

        for (int elem : seats) {
            std::cout << elem << ",";
        }

        std::cout << "\n";

        for (int elem : students) {
            std::cout << elem << ",";
        }
        
        int result = 0;
        for (std::size_t i = 0; i != seats.size(); ++i) {
            int a = seats[i];
            int b = students[i];
            result += abs(a - b);
        }

        return result;
    }
};


int main() {
    std::vector<int> seats{2,2,6,6};
    std::vector<int> students{1,3,2,6};

    int moves = Solution::minMovesToSeat(seats, students);
    std::cout << "\n" << moves << "\n";
}