#include <iostream>
#include <cstddef>
#include <cstdint>


struct Item {
    uint8_t day;
    uint8_t month;
    uint16_t year;
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
    : data_{new T[10]()}
    , size_{0}
    , capacity_{10}
    {}

    explicit MyVector(std::size_t n)
    : data_{new T[n]()}
    , size_{n}
    , capacity_{n}
    {}

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
        if (this == &other) {
            return *this;
        }

        delete[] data_;
        data_ = new T[other.capacity_];
        size_ = other.size_;
        capacity_ = other.capacity_;
        for (std::size_t i = 0; i != other.size_; ++i) {
            data_[i] = other.data_[i];
        }

        return *this;
    }

    MyVector& operator=(MyVector&& other) noexcept {
        data_ = std::move(other.data_);
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;

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

    void clear() {
        delete[] data_;
        data_ = new T{10};
        size_ = 0;
        capacity_ = 10;
    }

    T* data() { return data_; }
    const T* data() const { return data_; }
};


class RadixSort {
private:
    ~RadixSort() = delete;

    template <typename SortKey>
    static void stable_counting_sort(MyVector<Item>& vec, MyVector<Item>& buffer, SortKey key) {
        int max_k = 0;
        for (std::size_t i = 0; i != vec.size(); ++i) {
            int k = static_cast<int>(key(vec[i]));
            if (k > max_k) {
                max_k = k;
            }
        }

        int count[max_k + 1]{0};
        for (std::size_t i = 0; i != vec.size(); ++i) {
            ++count[static_cast<std::size_t>(key(vec[i]))];
        }
        
        for (int i = 1; i != max_k + 1; ++i) {
            count[i] += count[i - 1];
        }
        
        for (std::size_t i = vec.size(); i != 0; --i) {
            int k = static_cast<int>(key(vec[i - 1]));
            buffer[count[k] - 1] = vec[i - 1];
            --count[k];
        }
    }
    
    public:
    static void stable_radix_sort(MyVector<Item>& vec) {
        if (vec.size() == 0) {
            return;
        }

        MyVector<Item> buffer{vec.size()};
        stable_counting_sort(vec, buffer, [](const Item& itm) -> uint8_t { return itm.day; });
        stable_counting_sort(buffer, vec, [](const Item& itm) -> uint8_t { return itm.month; });
        stable_counting_sort(vec, buffer, [](const Item& itm) -> uint16_t { return itm.year; });

        vec = std::move(buffer);
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
    uint32_t pos = static_cast<uint32_t>(ind);
    uint32_t len = static_cast<uint32_t>(line.size());
    
    return Item{day, month, year, pos, len};
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
        // line.~MyVector();
    }
    
    RadixSort::stable_radix_sort(vec);
    
    for (auto& elem : vec) {
        std::cout.write(all_data.data() + elem.pos, elem.len);
        std::cout << "\n";
    }

    return 0;
}