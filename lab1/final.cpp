#include <iostream>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>

// 2 типа объектов, digitizer и order
// order знает radix и умеет маппить ключи на их порядковый номер: [0; radix)
//order принимает на вход ключ и возвращает size_t
//digitizer знает ширину ключей (общую для всех) и умеет доставать из ключа биты с индексами: [idx; idx + step)
//метод принимает на вход ключ idx и step и возвращается size_t
//order - обертка над digitizer, у которой есть еще idx и step
//radix обязан знать ордер - 1 << step
//в radix_sort передается digitizer, там он оборачивается order'ом и передается в counting_sort



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



template <typename T>
class Digitizer final{
private:
    std::size_t width_;

public:
    Digitizer()
    : width_{std::numeric_limits<T>::is_signed + std::numeric_limits<T>::digits}
    {
    }

    template <typename Iterator>
    std::size_t get_pos(Iterator it, std::size_t index, std::size_t step) const {
        auto value = it->date;
        return (value >> index) & ((T{1} << step) - T{1});
    }

    std::size_t get_width() const { return width_; }
};


class RadixSort final{
private:
    std::size_t step_ = 11;

public:
    RadixSort() = default;
    RadixSort(std::size_t step) : step_{step} {}

private: //for private classes
    template <typename Digitizer>
    class Order {
        private:
            std::size_t index_;
            std::size_t step_;
            Digitizer digit_;

        public:
            Order(std::size_t index, std::size_t step, Digitizer digit)
                : index_{index}
                , step_{step}
                , digit_{std::move(digit)}
            {
            }

            std::size_t get_index() const { return index_; }
            std::size_t get_width() const { return digit_.get_width(); }
            void update_index() { index_ += step_; }
            std::size_t radix() const { return 1 << step_; }

            template <typename Iterator>
            std::size_t get_pos(Iterator it) const {
                return digit_.get_pos(it, index_, step_);
            }

    };

private: //for private funcs
    template <typename InIter, typename OutIter, typename Digitizer>
    void counting_sort(
        InIter start_it, InIter end_it,
        OutIter out_it, Order<Digitizer> order)
    {
        MyVector<std::size_t> count(order.radix());
        
        for (auto it = start_it; it != end_it; ++it) {
            ++count[order.get_pos(it)];
        }

        for (std::size_t i = 1; i != order.radix(); ++i) {
            count[i] += count[i - 1];
        }

        auto rbegin = std::reverse_iterator(end_it);
        auto rend = std::reverse_iterator(start_it);
        for (auto it = rbegin; it != rend; ++it) {
            std::size_t pos = count[order.get_pos(it)] - 1;
            --count[order.get_pos(it)];
            out_it += pos;
            *out_it = *it;
            out_it -= pos;
        }
    }
    
public:
    template <typename Iterator, typename Digitizer>
    void radix_sort(Iterator start, Iterator end, Digitizer digit) {
        if (start == end) { return; }

        Order order(0, step_, digit);

        std::size_t size = static_cast<std::size_t>(std::distance(start, end));
        using T = typename std::iterator_traits<Iterator>::value_type;
        MyVector<T> buffer(size);

        bool sorted_in_buffer = false;
        auto buffer_start = buffer.begin();
        auto buffer_end = buffer.end();
        
        while (order.get_index() < order.get_width()) {
            
            if (!sorted_in_buffer) {
                counting_sort(start, end, buffer_start, order);
                sorted_in_buffer = true;
            } else {
                counting_sort(buffer_start, buffer_end, start, order);
                sorted_in_buffer = false;
            }

            order.update_index();
        }

        if (sorted_in_buffer) {
            auto buffer_it = buffer_start;
            for (auto it = start; it != end; ++it) {
                *it = *buffer_it++; 
            }
        }
    }
};



class Parser final {
private:
    template <typename T>
    static std::size_t find(const T& line, char c, std::size_t start = 0) noexcept {
        for (std::size_t i = start; i != line.size(); ++i) {
            if (line[i] == c) {
                return i;
            }
        }
        
        return -1;
    }
    
    template <typename Iterator>
    static int stoi(Iterator start, Iterator end, std::size_t shift = 0) noexcept {
        int result = 0;
        while (start + shift != end) {
            if (*(start + shift) >= '0' && *(start + shift) <= '9') {
                result = result * 10 + *(start + shift) - '0';
            } else {
                return result;
            }
            
            ++start;
        }
        
        return result;
    }
    
public:
    template <typename T>
    static Item parser(const T& line, std::size_t ind) noexcept {
        std::size_t pos1 = find(line, '.');
        std::size_t pos2 = find(line, '.', pos1 + 1);
        std::size_t pos3 = find(line, '\t');
        
        uint8_t day = static_cast<uint8_t>(stoi(line.begin(), line.end()));
        uint8_t month = static_cast<uint8_t>(stoi(line.begin() + pos1 + 1, line.end()));
        uint16_t year = static_cast<uint16_t>(stoi(line.begin() + pos2 + 1, line.end()));
        uint32_t date = static_cast<uint32_t>(year) * 12 * 31 +
                        static_cast<uint32_t>(month) * 31 + 
                        static_cast<uint32_t>(day);
        uint32_t pos = static_cast<uint32_t>(ind);
        uint32_t len = static_cast<uint32_t>(line.size());
        
        return Item{date, pos, len};
    }
};



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
                vec.push_back(Parser::parser(line, pos));
                line.clear();
                pos = all_data.size();
            }

        } else {
            all_data.push_back(c);
            line.push_back(c);
        }
    }

    if (line.size() > 0) {
        vec.push_back(Parser::parser(line, pos));
        line.clear();
    }
    
    Digitizer<std::uint32_t> digit;
    RadixSort rs;
    rs.radix_sort(vec.begin(), vec.end(), digit);
    
    for (auto& elem : vec) {
        std::cout.write(all_data.data() + elem.pos, elem.len);
        std::cout << "\n";
    }

    return 0;
}