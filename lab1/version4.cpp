#include <iostream>
#include <cstddef>
#include <cstdint>
#include <iterator>


struct Item {
    uint32_t date;
    uint32_t pos;
    uint32_t len;
};


template <typename T>
class MyVector {
private:
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

public:
    MyVector()
    : data_{new T[8]()}
    , size_{0}
    , capacity_{8}
    {}

    explicit MyVector(std::size_t n)
    : data_{new T[n]()}
    , size_{n}
    , capacity_{n}
    {}

    template <typename Iter>
    MyVector(Iter start, Iter end)
    : data_{nullptr}
    , size_{0}
    , capacity_{0}
    {
        auto dist = std::distance(start, end);
        data_ = new T[dist]();
        size_ = dist;
        capacity_ = dist;
        std::size_t idx = 0;
        for (auto it = start; it != end; ++it) {
            data_[idx++] = *it;
        }
    }

    MyVector(const MyVector& other)
    : data_{new T[other.capacity_]}
    , size_{other.size_}
    , capacity_{other.capacity_}
    {
        for (std::size_t i = 0; i != other.size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    MyVector(MyVector&& other) noexcept
    : data_{other.data_}
    , size_{other.size_}
    , capacity_{other.capacity_}
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    ~MyVector() noexcept {
        delete[] data_;
        size_ = 0;
        capacity_ = 0;
    }

    MyVector& operator=(const MyVector& other) {
        if (this != &other) {
            delete[] data_;

            data_ = new T[other.capacity_];
            size_ = other.size_;
            capacity_ = other.capacity_;
            for (std::size_t i = 0; i != other.size_; ++i) {
                data_[i] = other.data_[i];
            }
        }
            
        return *this;
    }

    MyVector& operator=(MyVector&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
            
        return *this;
    }

    T& operator[](std::size_t index) { return data_[index]; }
    const T& operator[](std::size_t index) const { return data_[index]; }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }

    void push_back(const T& value) {
        if (size_ == capacity_) {
            capacity_ = (capacity_ == 0 ? 1 : capacity_ * 2);
            T* new_data = new T[capacity_];
            for (size_t i = 0; i != size_; ++i) {
                new_data[i] = std::move(data_[i]);
            }

            delete[] data_;
            data_ = new_data;
        }

        data_[size_++] = value;
    }

    T* begin() { return data_; }
    const T* begin() const { return data_; }
    T* end() { return data_ + size_; }
    const T* end() const { return data_ + size_; }

    using reverse_iterator = std::reverse_iterator<T*>;
    using const_reverse_iterator = std::reverse_iterator<const T*>;
    
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    void clear() {
        delete[] data_;
        data_ = new T[1]();
        size_ = 0;
        capacity_ = 1;
    }

    T* data() { return data_; }
    const T* data() const { return data_; }
};


class Solution final{
private: //for supportive funcs
    ~Solution() = delete;

    template <typename T>
    static constexpr std::size_t get_digits_count(T num) noexcept {
        return sizeof(T) * 8;
    }

private: //for classes
    class Digitizer final{
    private:
        std::size_t idx_;
        std::size_t step_;
        std::size_t width_;

    public:
        Digitizer(std::size_t idx, std::size_t step, std::size_t width)
            : idx_{idx}
            , step_{step}
            , width_{width}
        {}

        std::size_t radix() const { return 1 << step_; }

        template <typename T>
        std::size_t get_pos(T value) const {
            return (value >> idx_) & ((T{1} << step_) - T{1}); 
        }
    };

private: //for private funcs
    template <typename InIter, typename OutIter>
    static void counting_sort(InIter start_it, InIter end_it,
                       OutIter out_it, Digitizer digit)
    {
        std::size_t count[digit.radix()]{};
        
        for (auto it = start_it; it != end_it; ++it) {
            ++count[digit.get_pos(it->date)];
        }

        for (std::size_t i = 1; i != digit.radix(); ++i) {
            count[i] += count[i - 1];
        }

        auto rbegin = std::reverse_iterator(end_it);
        auto rend = std::reverse_iterator(start_it);
        for (auto it = rbegin; it != rend; ++it) {
            std::size_t pos = count[digit.get_pos(it->date)] - 1;
            --count[digit.get_pos(it->date)];
            out_it += pos;
            *out_it = *it;
            out_it -= pos;
        }
    }
    
public:
    template <typename Iterator>
    static void radix_sort(Iterator start, Iterator end) {
        if (start == end) { return; }

        std::size_t size = static_cast<std::size_t>(std::distance(start, end));
        std::size_t idx = 0;
        std::size_t step = 8;
        std::size_t width = get_digits_count(start->date);

        using T = typename std::iterator_traits<Iterator>::value_type;

        MyVector<T> buffer(size);

        bool sorted_in_buffer = false;
        auto buffer_start = buffer.begin();
        auto buffer_end = buffer.end();
        
        while (idx < width) {
            Digitizer digit{idx, step, width};
            
            if (!sorted_in_buffer) {
                counting_sort(start, end, buffer_start, digit);
                sorted_in_buffer = true;
            } else {
                counting_sort(buffer_start, buffer_end, start, digit);
                sorted_in_buffer = false;
            }

            idx += step;
        }

        if (sorted_in_buffer) {
            auto buffer_it = buffer_start;
            for (auto it = start; it != end; ++it) {
                *it = *buffer_it++; 
            }
        }
    }
};


std::size_t find(const MyVector<char>& line, char c, std::size_t start = 0) {
    for (std::size_t i = start; i != line.size(); ++i) {
        if (line[i] == c) {
            return i;
        }
    }

    return -1;
}

MyVector<char> substr(const MyVector<char>& line, std::size_t start, std::size_t end) {
    MyVector<char> result;
    for (std::size_t i = start; i != start + end; ++i) {
        result.push_back(line[i]);
    }

    return result;
}

int stoi(const MyVector<char>& line) {
    int result = 0;
    for (std::size_t i = 0; i != line.size(); ++i) {
        result = result * 10 + (line[i] - '0');
    }

    return result;
}

Item parser(const MyVector<char>& line, std::size_t ind) {
    std::size_t pos1 = find(line, '.');
    std::size_t pos2 = find(line, '.', pos1 + 1);
    std::size_t pos3 = find(line, '\t');
    
    uint8_t day = static_cast<uint8_t>(stoi(substr(line, 0, pos1)));
    uint8_t month = static_cast<uint8_t>(stoi(substr(line, pos1 + 1, pos2 - pos1 - 1)));
    uint16_t year = static_cast<uint16_t>(stoi(substr(line, pos2 + 1, pos3 - pos2 - 1)));
    uint32_t date = static_cast<uint32_t>(year) * 10000 +
                    static_cast<uint32_t>(month) * 100 + 
                    static_cast<uint32_t>(day);
    uint32_t pos = static_cast<uint32_t>(ind);
    uint32_t len = static_cast<uint32_t>(line.size());
    
    return Item{date, pos, len};
}


int main() {
    MyVector<Item> vec;
    MyVector<char> all_data;
    uint32_t pos = 0;

    char c;
    MyVector<char> line;
    bool line_feed_flag = false;
    while (std::cin.get(c)) {
        if (c == '\n') {
            if (line.size() > 0) {
                vec.push_back(parser(line, pos));
                line.clear();
                pos = all_data.size();
            }

        } else {
            all_data.push_back(c);
            line.push_back(c);
        }
    }

    if (line.size() > 0) {
        vec.push_back(parser(line, pos));
        line.clear();
    }

    Solution::radix_sort(vec.begin(), vec.end());
    
    for (auto& elem : vec) {
        std::cout.write(all_data.data() + elem.pos, elem.len);
        std::cout << "\n";
    }

    return 0;
}