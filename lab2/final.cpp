#include <cstddef>
#include <bit>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <utility> // for std::swap()


namespace mylib {
    template <typename T>
    class vector {
    private:
        T* data_;
        std::size_t size_;
        std::size_t capacity_;

    void resize_capacity(std::size_t new_cap) {
        T* new_data = new T[new_cap];
        for (std::size_t i = 0; i < size_; ++i) { new_data[i] = data_[i]; }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_cap;
    }

    public:
        vector() : data_(nullptr), size_(0), capacity_(0) {}
        vector(std::size_t n) : data_(nullptr), size_(n), capacity_(n) {
            if (n > 0) {
                data_ = new T[n];
                for (size_t i = 0; i < n; ++i) {
                    data_[i] = T();
                }
            }
        }

        ~vector() { delete[] data_; }

        vector(const vector& other) : data_(nullptr), size_(other.size_), capacity_(other.capacity_) {
            if (capacity_ > 0) {
                data_ = new T[capacity_];
                for (std::size_t i = 0; i < size_; ++i) {
                    data_[i] = other.data_[i];
                }
            }
        }

        vector& operator=(const vector& other) {
            if (this == &other) return *this;

            delete[] data_;

            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = (capacity_ > 0) ? new T[capacity_] : nullptr;

            for (std::size_t i = 0; i < size_; ++i) {
                data_[i] = other.data_[i];
            }

            return *this;
        }

        vector(vector&& other) noexcept
            : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }

        vector& operator=(vector&& other) noexcept {
            if (this == &other) return *this;

            delete[] data_;

            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;

            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;

            return *this;
        }

        void push_back(const T& value) {
            if (size_ == capacity_) {
                std::size_t new_cap = (capacity_ == 0) ? 1 : capacity_ * 2;
                resize_capacity(new_cap);
            }

            data_[size_++] = value;
        }

        void reserve(size_t new_cap) {
            if (new_cap <= capacity_) {
                return;
            }

            T* new_data = new T[new_cap];

            for (size_t i = 0; i < size_; ++i) {
                new_data[i] = data_[i];
            }

            delete[] data_;
            data_ = new_data;
            capacity_ = new_cap;
        }

        void pop_back() { if (size_ > 0) { --size_; } }
        std::size_t size() const { return size_; }
        void clear() { size_ = 0; }
        T& operator[](std::size_t index) { return data_[index]; }
        const T& operator[](std::size_t index) const { return data_[index]; }
        T& back() { return data_[size_ - 1]; }
        const T& back() const { return data_[size_ - 1]; }
        T& front() { return data_[0]; }
        const T& front() const { return data_[0]; }
        T* data() { return data_; }
        const T* data() const { return data_; }
        T* begin() { return data_; }
        const T* begin() const { return data_;}
        T* end() { return data_ + size_; }
        const T* end() const { return data_ + size_; }
    };

    char toLower(char c) {
        if (c >= 'A' && c <= 'Z') { return c + ('a' - 'A'); }
        return c;
    }
}


class Digitizer final {
private:
    static constexpr std::size_t charBits{5};
public:
    bool operator()(
        const mylib::vector<char>& str,
        const std::ptrdiff_t index
    ) const noexcept {
        if (index == -1) { return false; }
        if (index >= str.size() * charBits) { return false; }
        return bool(str[str.size() - index / charBits - 1] >> (index % charBits) & 1);
    }

    std::ptrdiff_t operator()(
        const mylib::vector<char>& first,
        const mylib::vector<char>& second
    ) const noexcept {

        const std::size_t lengthFirst = first.size();
        const std::size_t lengthSecond = second.size();
        if (lengthFirst > lengthSecond) return (*this)(second, first);

        const std::size_t diff = lengthSecond - lengthFirst;
        for (std::size_t i = lengthFirst; i > 0; --i) {
            if (mylib::toLower(first[i - 1]) != mylib::toLower(second[i + diff - 1])) {
                return std::countr_zero(static_cast<unsigned int>(first[i - 1] ^ second[i + diff - 1])) + charBits * (lengthFirst - i);
            }
        }

        if (lengthFirst == lengthSecond) { return -1; }
        return std::countr_zero(static_cast<unsigned int>(second[diff - 1])) + charBits * lengthFirst;
    }
};


template <typename Key, typename T, typename Digitizer>
class PatriciaTrie final {
private:
    struct Node final {
    public:
        Node *right_{nullptr}, *left_{nullptr};
        Key key_{};
        T value_{};
        std::ptrdiff_t bit_{-1};

        std::ptrdiff_t queue_index{-1};
        
        Node() = default;
        Node(const Key& key, const T& value)
            : key_{key}
            , value_{value}
        {
        }

        const Key& get_key() const { return key_; }
        const T& get_value() const { return value_; }
        void set_key(const Key& key) { key_ = key; }
        void set_value(const T& value) { value_ = value; }
    };

    Node* root_{nullptr};
    std::size_t size_{0};
    Digitizer digitizer_{};

private:
    const T* find_(const Key& key) const {
        if (!root_) return nullptr;
        Node* currunt{root_->left_};
        std::ptrdiff_t prev_bit{-1};
        std::ptrdiff_t diff_bit{-1};
        while (true) {
            if (prev_bit >= currunt->bit_) {
                diff_bit = digitizer_(key, currunt->get_key());
                if (diff_bit == -1) { return &currunt->get_value(); }
                break;
            }

            prev_bit = currunt->bit_;
            currunt = (digitizer_(key, currunt->bit_) == 0) ? currunt->left_ : currunt->right_;
        }

        return nullptr;
    }

    bool insert_(const Key& key, const T& value) {
        if (!root_) {
            Node* new_node = new Node(key, value);
            root_ = new_node;
            root_->left_ = root_;
            ++size_;
            return true;
        }

        Node* currunt{root_->left_};
        std::ptrdiff_t prev_bit{-1};
        std::ptrdiff_t diff_bit{-1};
        while (true) {
            if (prev_bit >= currunt->bit_) {
                diff_bit = digitizer_(key, currunt->get_key());
                if (diff_bit == -1) { return false; }
                break;
            }

            prev_bit = currunt->bit_;
            currunt = (digitizer_(key, currunt->bit_) == 0) ? currunt->left_ : currunt->right_;
        }

        Node* insert_node{root_};
        Node* next{nullptr};
        while (true) {
            std::ptrdiff_t bit_choice{digitizer_(key, insert_node->bit_)};
            next = (bit_choice == 0) ? insert_node->left_ : insert_node->right_;
            if (diff_bit > next->bit_ && insert_node->bit_ < next->bit_) {
                insert_node = next;
            } else { break; }
        }

        Node* new_node = new Node(key, value);
        if (digitizer_(new_node->get_key(), insert_node->bit_) == 0) {
            insert_node->left_ = new_node;
        } else { insert_node->right_ = new_node; }
        
        if (digitizer_(new_node->get_key(), diff_bit) == 0) {
            new_node->left_ = new_node;
            new_node->right_ = next;
        } else {
            new_node->right_ = new_node;
            new_node->left_ = next;
        }
        
        new_node->bit_ = diff_bit;
        ++size_;
        return true;
    }

    bool erase_(const Key& key) {
        if (size_ == 0) { return false; }
        if (digitizer_(key, root_->get_key()) == -1 && size_ == 1) {
            delete root_;
            root_ = nullptr;
            size_ = 0;
            return true;
        }
        
        Node *upParent = nullptr;
        Node *upToDelete = root_;
        Node *deleteNode = root_->left_;

        do {
            upParent = upToDelete;
            upToDelete = deleteNode;
            deleteNode = digitizer_(key, deleteNode->bit_)
                ? deleteNode->right_
                : deleteNode->left_;
        } while (deleteNode->bit_ > upToDelete->bit_);

        if (digitizer_(deleteNode->get_key(), key) != -1) return false;

        Node *upToUp = nullptr;
        Node *temp = upToDelete;

        do {
            upToUp = temp;
            temp = digitizer_(upToDelete->get_key(), temp->bit_) 
                ? temp->right_
                : temp->left_;
        } while (temp->bit_ > upToUp->bit_);

        if (upToUp->left_ == upToDelete) upToUp->left_ = deleteNode;
        else upToUp->right_ = deleteNode;

        temp = (upToDelete->left_ == deleteNode) ? upToDelete->right_ : upToDelete->left_;
        if (upParent->left_ == upToDelete) upParent->left_ = temp;
        else upParent->right_ = temp;

        std::swap(deleteNode->key_, upToDelete->key_);
        std::swap(deleteNode->value_, upToDelete->value_);

        delete upToDelete;
        --size_;
        return true;
    }

    void clear_() {
        if (!root_) return;

        mylib::vector<Node*> nodes;
        nodes.push_back(root_);
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            Node* node{nodes[i]};
            if (node->left_ && node->left_->bit_ > node->bit_) { nodes.push_back(node->left_); }
            if (node->right_ && node->right_->bit_ > node->bit_) { nodes.push_back(node->right_); }
        }

        for (std::size_t i = 0; i < nodes.size(); ++i) {
            delete nodes[i];
        }

        root_ = nullptr;
        size_ = 0;
        return;
    }

    bool save_(const char* file_path) {
        std::ofstream out(file_path, std::ios::binary);
        out.write(reinterpret_cast<const char*>(&size_), sizeof(size_));
        if (!root_) {
            out.close();
            return true;
        }

        mylib::vector<Node*> queue{};
        root_->queue_index = 0;
        queue.push_back(root_);
        std::ptrdiff_t counter{0};
        for (std::size_t i = 0; i < queue.size(); ++i) {
            Node* node{queue[counter]};
            if (node->left_ && node->left_->bit_ > node->bit_) {
                node->left_->queue_index = queue.size();
                queue.push_back(node->left_);
            }

            if (node->right_ && node->right_->bit_ > node->bit_) {
                node->right_->queue_index = queue.size();
                queue.push_back(node->right_);
            }

            ++counter;
        }
        
        for (std::size_t i = 0; i < queue.size(); ++i) {
            Node* node{queue[i]};

            const Key& key = node->get_key();
            std::size_t key_len = key.size();
            const T& value = node->get_value();
            std::ptrdiff_t left_child{-1}, right_child{-1};
            if (node->left_) left_child = node->left_->queue_index;
            if (node->right_) right_child = node->right_->queue_index;

            out.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
            out.write(key.data(), key_len);
            out.write(reinterpret_cast<const char*>(&value), sizeof(value));
            out.write(reinterpret_cast<const char*>(&node->bit_), sizeof(node->bit_));
            out.write(reinterpret_cast<char*>(&left_child), sizeof(left_child));
            out.write(reinterpret_cast<char*>(&right_child), sizeof(right_child));
        }

        out.close();
        return true;
    }

    bool load_(const char* file_path) {
        std::ifstream input(file_path, std::ios::binary);

        clear_();
        std::size_t trie_size{};
        input.read(reinterpret_cast<char*>(&trie_size), sizeof(trie_size));
        if (!trie_size) {
            root_ = nullptr;
            size_ = 0;
            return true;
        }

        mylib::vector<Node*> queue{};
        mylib::vector<std::ptrdiff_t> left_childs{};
        mylib::vector<std::ptrdiff_t> right_childs{};
        for (std::size_t i = 0; i < trie_size; ++i) {
            Node* node = new Node{};
            
            std::size_t key_len{};
            input.read(reinterpret_cast<char*>(&key_len), sizeof(key_len));

            Key key(key_len);
            T value{};
            std::ptrdiff_t bit{};
            std::ptrdiff_t left_child{}, right_child{};

            input.read(reinterpret_cast<char*>(key.data()), key_len);
            input.read(reinterpret_cast<char*>(&value), sizeof(value));
            input.read(reinterpret_cast<char*>(&bit), sizeof(bit));

            node->set_key(key);
            node->set_value(value);
            node->bit_ = bit;
            queue.push_back(node);

            input.read(reinterpret_cast<char*>(&left_child), sizeof(left_child));
            input.read(reinterpret_cast<char*>(&right_child), sizeof(right_child));

            left_childs.push_back(left_child);
            right_childs.push_back(right_child);
        }

        for (std::size_t i = 0; i < queue.size(); ++i) {
            Node* node = queue[i];
            std::ptrdiff_t left_child{left_childs[i]};
            if (left_child != -1) node->left_ = queue[left_child];
            std::ptrdiff_t right_child{right_childs[i]};
            if (right_child != -1) node->right_ = queue[right_child];
        }

        root_ = queue[0];
        size_ = trie_size;
        return true;
    }
        
public:
    PatriciaTrie(const Digitizer& digitizer)
        : digitizer_{digitizer}
    {
    }

    ~PatriciaTrie() { clear_(); }

    const T* find(const Key& key) const { return find_(key); }
    bool insert(const Key& key, const T& value) { return insert_(key, value); }
    bool erase(const Key& key) { return erase_(key); }
    bool save(const char* file_path) { return save_(file_path); }
    bool load(const char* file_path) { return load_(file_path); }
    std::size_t size() const { return size_; }
    void clear() { return clear_(); }
};


bool is_whitespace(char c) {
    return (c == ' ' || c == '\n');
}

bool read_word(mylib::vector<char>& vec) {
    vec.clear();
    char c;
    while (std::cin.get(c)) {
        if (!is_whitespace(c)) {
            vec.push_back(c);
            break;
        }
    }

    if (vec.size() == 0) return false;

    while (std::cin.get(c) && !is_whitespace(c)) {
        vec.push_back(c);
    }

    return true;
}

void normalize_key(mylib::vector<char>& key) {
    for (std::size_t i = 0; i < key.size(); ++i) {
        key[i] = mylib::toLower(key[i]);
    }
}

int main() {
    Digitizer digitizer{};
    PatriciaTrie<mylib::vector<char>, std::uint64_t, Digitizer> patricia(digitizer);
    mylib::vector<char> command{}, word{}, path{};
    std::uint64_t value;

    while (read_word(command)) {
        if (command.size() == 1 && command[0] == '+') {
            read_word(word);
            std::cin >> value;
            std::cin.ignore();

            normalize_key(word);
            if (patricia.insert(word, value)) std::cout << "OK\n";
            else std::cout << "Exist\n";

        } else if (command.size() == 1 && command[0] == '-') {
            read_word(word);
            normalize_key(word);
            if (patricia.erase(word)) std::cout << "OK\n";
            else std::cout << "NoSuchWord\n";

        } else if (command.size() == 1 && command[0] == '!') {
            read_word(word);
            read_word(path);
            path.push_back('\0');

            const char save_str[] = {'S','a','v','e'};
            const char load_str[] = {'L','o','a','d'};
            bool is_save{true};
            bool is_load{true};
            for (std::size_t i = 0; i < 4; ++i){
                if (word[i] != save_str[i]) {
                    is_save = false;
                    break;
                }
            }

            for (std::size_t i = 0; i < 4; ++i) {
                if (word[i] != load_str[i]) {
                    is_load = false;
                    break;
                }
            }

            if (is_save) {
                if (patricia.save(path.data()))
                    std::cout << "OK\n";
            } else if (is_load) {
                if (patricia.load(path.data()))
                    std::cout << "OK\n";
            }

        } else {
            normalize_key(command);
            const std::uint64_t* result = patricia.find(command);
            if (result) std::cout << "OK: " << *result << '\n';
            else std::cout << "NoSuchWord\n";
        }
    }
}