#define _CRT_SECURE_NO_WARNINGS 
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <string>

//                                    Клас для роботи з рядками без витоків пам'яті та зациклень
class MyString
{
    char* data;

public:
    //                               Стандартний конструктор
    MyString(const char* s = "")
    {
        if (s == nullptr) {
            data = new char[1];
            data[0] = '\0';
        }
        else {
            data = new char[strlen(s) + 1];
            strcpy(data, s);
        }
    }

    //                                Конструктор копіювання
    MyString(const MyString& other)
    {
        data = new char[strlen(other.data) + 1];
        strcpy(data, other.data);
    }

    //                                   Оператор присвоювання копіюванням
    MyString& operator=(const MyString& other)
    {
        if (this != &other)
        {
            char* new_data = new char[strlen(other.data) + 1];
            strcpy(new_data, other.data);
            delete[] data;
            data = new_data;
        }
        return *this;
    }

    //                             Конструктор переміщення (Запобігає Double Free у векторах)
    MyString(MyString&& other) noexcept
    {
        data = other.data;
        other.data = nullptr; //               Забираємо володіння пам'яттю
    }

    //                                       Оператор присвоювання переміщення
    MyString& operator=(MyString&& other) noexcept
    {
        if (this != &other)
        {
            delete[] data;
            data = other.data;
            other.data = nullptr;
        }
        return *this;
    }

    //                                       Деструктор
    ~MyString()
    {
        delete[] data;
    }

    bool operator<(const MyString& other) const { return strcmp(data, other.data) < 0; }
    bool operator==(const MyString& other) const
    {
        if (data == nullptr || other.data == nullptr) return data == other.data;
        return strcmp(data, other.data) == 0;
    }

    bool empty() const
    {
        return data == nullptr || strlen(data) == 0;
    }

    const char* c_str() const
    {
        return data ? data : "";
    }

    //                                        Поліноміальна хеш-функція для рядків
    int hash(int a, int b, int p, int m) const
    {
        if (m <= 0 || data == nullptr) return 0;
        long long h = 0;
        for (int i = 0; data[i] != '\0'; ++i)
        {
            h = (h * a + data[i]) % p;
        }
        return static_cast<int>(((h + b) % p) % m);
    }
};

//                                                2-Й РІВЕНЬ ХЕШУВАННЯ 

struct SecondLevelTable
{
    int hash_a = 0, hash_b = 0, prime_p = 0, table_size_m = 0;
    std::vector<MyString> cells;
    bool exists = false;

    void build(const std::vector<MyString>& keys, int p_val)
    {
        if (keys.empty()) return;

        prime_p = p_val;
        table_size_m = static_cast<int>(keys.size() * keys.size()); //              розмір m_i = c_i^2
        cells.assign(table_size_m, MyString(""));
        exists = true;

        if (table_size_m == 1)
        {
            hash_a = 1; hash_b = 0;
            cells[0] = keys[0];
            return;
        }

        //                                                   циклічний підбір параметрів до повної відсутності колізій
        while (true)
        {
            hash_a = rand() % (prime_p - 1) + 1;
            hash_b = rand() % prime_p;

            std::fill(cells.begin(), cells.end(), MyString(""));
            bool collision = false;

            for (const auto& key : keys)
            {
                int idx = key.hash(hash_a, hash_b, prime_p, table_size_m);
                if (!cells[idx].empty())
                {
                    collision = true; //                              Знайдено колізію, перезапускаємо генерацію a та b
                    break;
                }
                cells[idx] = key;
            }

            if (!collision) break; //                              Слот заповнено  без колізій
        }
    }

    int search(const MyString& key) const
    {
        if (!exists || table_size_m == 0) return -1;
        int idx = key.hash(hash_a, hash_b, prime_p, table_size_m);
        if (!cells[idx].empty() && cells[idx] == key) return idx;
        return -1;
    }
};

//                                              1-Й РІВЕНЬ ХЕШУВАННЯ 

struct PrimaryLevelTable
{
    int hash_a = 0, hash_b = 0, total_buckets_M = 0;
    int prime_p = 0;
    std::vector<SecondLevelTable> buckets;

    void init(int size, int p_val)
    {
        total_buckets_M = size;
        prime_p = p_val;
        buckets.resize(total_buckets_M);

        hash_a = rand() % (prime_p - 1) + 1;
        hash_b = rand() % prime_p;
    }

    int get_bucket_index(const MyString& key) const
    {
        return key.hash(hash_a, hash_b, prime_p, total_buckets_M);
    }
};


class PerfectHashMap
{
    int prime_p = 1000003; //                             Велике просте число
    PrimaryLevelTable first_level;

public:
    PerfectHashMap(const std::vector<MyString>& all_keys)
    {
        int M = static_cast<int>(all_keys.size());
        if (M == 0) return;

        //                                        Ініціалізація першого рівня
        first_level.init(M, prime_p);

        //                                       Розподіл ключів
        std::vector<std::vector<MyString>> temp_buckets(M);
        for (const auto& key : all_keys)
        {
            int bucket_idx = first_level.get_bucket_index(key);
            temp_buckets[bucket_idx].push_back(key);
        }

        //                                          Побудова другого рівня для кожного бакета
        for (int i = 0; i < M; ++i)
        {
            first_level.buckets[i].build(temp_buckets[i], prime_p);
        }
    }

    //                                                Дворівневий пошук за константний час
    bool search(const MyString& key, int& out_bucket, int& out_cell) const
    {
        if (first_level.total_buckets_M == 0) return false;

        int bucket_idx = first_level.get_bucket_index(key);
        int cell_idx = first_level.buckets[bucket_idx].search(key);

        if (cell_idx != -1)
        {
            out_bucket = bucket_idx;
            out_cell = cell_idx;
            return true;
        }
        return false;
    }

                                            
    void print_structure() const
    {
        std::cout << "\n=============================================\n";
        std::cout << "PRIMARY LEVEL TABLE STRUCTURE:\n";
        std::cout << "Global parameters: M = " << first_level.total_buckets_M
            << ", a = " << first_level.hash_a << ", b = " << first_level.hash_b << "\n";
        std::cout << "=============================================\n";

        std::cout << "\nSECONDARY LEVEL BUCKETS STRUCTURE:\n";
        for (int i = 0; i < first_level.total_buckets_M; ++i)
        {
            std::cout << "Bucket [" << i << "]: ";
            if (!first_level.buckets[i].exists)
            {
                std::cout << "empty\n";
            }
            else
            {
                std::cout << "(m_i=" << first_level.buckets[i].table_size_m
                    << ", a_i=" << first_level.buckets[i].hash_a
                    << ", b_i=" << first_level.buckets[i].hash_b << ") -> ";
                for (int j = 0; j < first_level.buckets[i].table_size_m; ++j)
                {
                    if (!first_level.buckets[i].cells[j].empty())
                        std::cout << "[" << j << ":" << first_level.buckets[i].cells[j].c_str() << "] ";
                    else
                        std::cout << "[" << j << ":-] ";
                }
                std::cout << "\n";
            }
        }
    }
};

int main()
{
    //                                               Ініціалізація рандому часу
    srand(static_cast<unsigned int>(time(0)));

    std::cout << "Perfect Hashing of Strings (Variant 8)\n";

    int n;
    std::cout << "Enter the number of elements: ";
    if (!(std::cin >> n) || n <= 0) return 0;

    std::vector<MyString> input_strings;
    input_strings.reserve(n);
    std::cout << "Enter " << n << " strings:\n";
    for (int i = 0; i < n; ++i)
    {
        std::string s;
        std::cout << "String " << i + 1 << ": ";
        std::cin >> s;
        input_strings.push_back(MyString(s.c_str()));
    }

    //                                              Побудова ідеального хешування
    PerfectHashMap hash_map(input_strings);

    //                                             Виведення структури 1-го та 2-го рівнів
    hash_map.print_structure();

    //                                              Перевірка пошуку за O(1)
    std::cout << "\n--- Search Testing (O(1) Execution Time) ---\n";
    char choice = 'y';
    while (choice == 'y' || choice == 'Y')
    {
        std::string search_str;
        std::cout << "Enter a string to search: ";
        std::cin >> search_str;

        int bucket = -1, cell = -1;
        if (hash_map.search(MyString(search_str.c_str()), bucket, cell))
        {
            std::cout << "Success! String found.\n";
            std::cout << "Position: Primary Level Bucket [" << bucket << "], Secondary Level Cell [" << cell << "]\n";
        }
        else
        {
            std::cout << "String NOT found.\n";
        }

        std::cout << "Search another string? (y/n): ";
        std::cin >> choice;
    }
    return 0;
}