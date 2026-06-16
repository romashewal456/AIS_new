#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>

//  визначення кольорів для дерева
enum class Color { RED, BLACK };


// КЛАС РАЦІОНАЛЬНИХ ЧИСЕЛ 

class Rational
{
private:

    static long long getGcd(long long a, long long b)
    {
        while (b != 0)
        {
            long long temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    static long long getLcm(long long a, long long b)
    {
        if (a == 0 || b == 0) return 0;
        return (a / getGcd(a, b)) * b;
    }

public:
    long long num, den;

    Rational(long long n = 0, long long d = 1)
    {
        if (d == 0) d = 1; // Захист від ділення на нуль
        long long common = getGcd(std::abs(n), std::abs(d));
        num = (d < 0 ? -n : n) / common;
        den = std::abs(d) / common;
    }

    // Оператори порівняння
    bool operator<(const Rational& other) const { return num * other.den < other.num * den; }
    bool operator>(const Rational& other) const { return other < *this; }
    bool operator<=(const Rational& other) const { return !(*this > other); }
    bool operator>=(const Rational& other) const { return !(*this < other); }
    bool operator==(const Rational& other) const { return num == other.num && den == other.den; }

    // Оператор додавання 
    Rational operator+(const Rational& other) const
    {
        long long common_den = getLcm(den, other.den);
        long long new_num = num * (common_den / den) + other.num * (common_den / other.den);
        return Rational(new_num, common_den);
    }

    // Виведення у потік
    friend std::ostream& operator<<(std::ostream& os, const Rational& r)
    {
        if (r.den == 1) return os << r.num;
        return os << r.num << "/" << r.den;
    }
};


//  В-ДЕРЕВО 

template <int T>
class BTree
{
private:
    struct Node
    {
        Rational keys[2 * T - 1];
        Node* children[2 * T];
        int n = 0;
        bool leaf = true;

        Node(bool isLeaf) : leaf(isLeaf)
        {
            for (int i = 0; i < 2 * T; i++) children[i] = nullptr;
        }
    };

    Node* root;

    // Допоміжний метод для очищення пам'яті
    void clear(Node* node)
    {
        if (!node) return;
        if (!node->leaf)
        {
            for (int i = 0; i <= node->n; i++)
                clear(node->children[i]);
        }
        delete node;
    }

    // Розщеплення повного дочірнього вузла
    void splitChild(Node* x, int i, Node* y)
    {
        Node* z = new Node(y->leaf);
        z->n = T - 1;
        for (int j = 0; j < T - 1; j++)
            z->keys[j] = y->keys[j + T];

        if (!y->leaf)
        {
            for (int j = 0; j < T; j++)
                z->children[j] = y->children[j + T];
        }
        y->n = T - 1;

        for (int j = x->n; j >= i + 1; j--)
            x->children[j + 1] = x->children[j];

        x->children[i + 1] = z;

        for (int j = x->n - 1; j >= i; j--)
            x->keys[j + 1] = x->keys[j];

        x->keys[i] = y->keys[T - 1];
        x->n++;
    }

    // Вставка у неповний вузол
    void insertNonFull(Node* x, Rational k)
    {
        int i = x->n - 1;
        if (x->leaf)
        {
            while (i >= 0 && x->keys[i] > k)
            {
                x->keys[i + 1] = x->keys[i];
                i--;
            }
            x->keys[i + 1] = k;
            x->n++;
        }
        else
        {
            while (i >= 0 && x->keys[i] > k)
                i--;
            if (x->children[i + 1]->n == 2 * T - 1)
            {
                splitChild(x, i + 1, x->children[i + 1]);
                if (x->keys[i + 1] < k)
                    i++;
            }
            insertNonFull(x->children[i + 1], k);
        }
    }

    // Рекурсивний пошук вузла
    bool searchNode(Node* x, Rational k) const
    {
        int i = 0;
        while (i < x->n && k > x->keys[i])
            i++;
        if (i < x->n && x->keys[i] == k)
            return true;
        if (x->leaf)
            return false;
        return searchNode(x->children[i], k);
    }

    // Рекурсивне виведення структури дерева
    void traverse(Node* x, int depth) const
    {
        if (!x) return;
        std::cout << std::string(depth * 4, ' ') << "[ ";
        for (int i = 0; i < x->n; i++)
        {
            std::cout << x->keys[i] << (i == x->n - 1 ? "" : ", ");
        }
        std::cout << " ]" << (x->leaf ? " (Leaf)" : "") << "\n";

        if (!x->leaf)
        {
            for (int i = 0; i <= x->n; i++)
                traverse(x->children[i], depth + 1);
        }
    }

public:
    BTree() : root(nullptr) {}
    ~BTree() { clear(root); }

    void insert(Rational k)
    {
        if (!root)
        {
            root = new Node(true);
            root->keys[0] = k;
            root->n = 1;
        }
        else
        {
            if (root->n == 2 * T - 1)
            {
                Node* s = new Node(false);
                s->children[0] = root;
                splitChild(s, 0, root);
                int i = (s->keys[0] < k) ? 1 : 0;
                insertNonFull(s->children[i], k);
                root = s;
            }
            else
                insertNonFull(root, k);
        }
    }

    bool search(Rational k) const
    {
        return root ? searchNode(root, k) : false;
    }

    void print() const
    {
        if (!root) std::cout << "B-Tree is empty.\n";
        else traverse(root, 0);
    }
};

// 3. ДЕРЕВО ВІДРІЗКІВ НА ЧБ-ДЕРЕВІ

struct RBNode
{
    Rational key;   // Значення ключа вузла
    Rational sum;   // Сума всього піддерева з цим коренем 
    Color color;
    RBNode* left, * right, * parent;

    RBNode(Rational k) : key(k), sum(k), color(Color::RED),
        left(nullptr), right(nullptr), parent(nullptr) {
    }
};

class SegmentRBTree
{
private:
    RBNode* root;
    RBNode* TNULL; // Фіктивний чорний лист

    // Оновлення  значення суми піддерева
    void updateSum(RBNode* x)
    {
        if (x == TNULL || x == nullptr) return;

        x->sum = x->key;
        if (x->left != TNULL && x->left != nullptr)
            x->sum = x->sum + x->left->sum;
        if (x->right != TNULL && x->right != nullptr)
            x->sum = x->sum + x->right->sum;
    }

    // Рекурсивний підйом вгору для оновлення сум після модифікацій
    void updateSumsUpward(RBNode* node)
    {
        while (node != nullptr && node != TNULL)
        {
            updateSum(node);
            node = node->parent;
        }
    }

    // Ліве обертання
    void leftRotate(RBNode* x)
    {
        RBNode* y = x->right;
        x->right = y->left;
        if (y->left != TNULL)
            y->left->parent = x;
        y->parent = x->parent;
        if (!x->parent)
            root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
        y->left = x;
        x->parent = y;

        updateSum(x);
        updateSum(y);
    }

    // Праве обертання
    void rightRotate(RBNode* x)
    {
        RBNode* y = x->left;
        x->left = y->right;
        if (y->right != TNULL)
            y->right->parent = x;
        y->parent = x->parent;
        if (!x->parent)
            root = y;
        else if (x == x->parent->right)
            x->parent->right = y;
        else
            x->parent->left = y;
        y->right = x;
        x->parent = y;

        updateSum(x);
        updateSum(y);
    }

    // Балансування ЧБ-дерева після вставки нового елемента
    void fixInsert(RBNode* k)
    {
        RBNode* u;
        while (k->parent && k->parent->color == Color::RED)
        {
            if (k->parent == k->parent->parent->right)
            {
                u = k->parent->parent->left; // Дядько вузла k
                if (u && u->color == Color::RED)
                {
                    u->color = Color::BLACK;
                    k->parent->color = Color::BLACK;
                    k->parent->parent->color = Color::RED;
                    k = k->parent->parent;
                }
                else
                {
                    if (k == k->parent->left)
                    {
                        k = k->parent;
                        rightRotate(k);
                    }
                    k->parent->color = Color::BLACK;
                    k->parent->parent->color = Color::RED;
                    leftRotate(k->parent->parent);
                }
            }
            else
            {
                u = k->parent->parent->right; // Дядько вузла k
                if (u && u->color == Color::RED)
                {
                    u->color = Color::BLACK;
                    k->parent->color = Color::BLACK;
                    k->parent->parent->color = Color::RED;
                    k = k->parent->parent;
                }
                else
                {
                    if (k == k->parent->right)
                    {
                        k = k->parent;
                        leftRotate(k);
                    }
                    k->parent->color = Color::BLACK;
                    k->parent->parent->color = Color::RED;
                    rightRotate(k->parent->parent);
                }
            }
            if (k == root) break;
        }
        root->color = Color::BLACK;
    }

    // Пошук мінімального вузла в піддереві для операції видалення
    RBNode* minimum(RBNode* node)
    {
        while (node->left != TNULL)
        {
            node = node->left;
        }
        return node;
    }

    // Заміна одного піддерева іншим при видаленні
    void rbTransplant(RBNode* u, RBNode* v)
    {
        if (u->parent == nullptr)
            root = v;
        else if (u == u->parent->left)
            u->parent->left = v;
        else
            u->parent->right = v;
        v->parent = u->parent;
    }

    // Балансування після видалення вузла 
    void fixDelete(RBNode* x)
    {
        RBNode* s;
        while (x != root && x->color == Color::BLACK)
        {
            if (x == x->parent->left)
            {
                s = x->parent->right; // Брат вузла x
                if (s->color == Color::RED)
                {
                    s->color = Color::BLACK;
                    x->parent->color = Color::RED;
                    leftRotate(x->parent);
                    s = x->parent->right;
                }

                if (s->left->color == Color::BLACK && s->right->color == Color::BLACK)
                {
                    s->color = Color::RED;
                    x = x->parent;
                }
                else
                {
                    if (s->right->color == Color::BLACK)
                    {
                        s->left->color = Color::BLACK;
                        s->color = Color::RED;
                        rightRotate(s);
                        s = x->parent->right;
                    }

                    s->color = x->parent->color;
                    x->parent->color = Color::BLACK;
                    s->right->color = Color::BLACK;
                    leftRotate(x->parent);
                    x = root;
                }
            }
            else
            {
                s = x->parent->left; // Брат вузла x
                if (s->color == Color::RED)
                {
                    s->color = Color::BLACK;
                    x->parent->color = Color::RED;
                    rightRotate(x->parent);
                    s = x->parent->left;
                }

                if (s->right->color == Color::BLACK && s->left->color == Color::BLACK)
                {
                    s->color = Color::RED;
                    x = x->parent;
                }
                else
                {
                    if (s->left->color == Color::BLACK)
                    {
                        s->right->color = Color::BLACK;
                        s->color = Color::RED;
                        leftRotate(s);
                        s = x->parent->left;
                    }

                    s->color = x->parent->color;
                    x->parent->color = Color::BLACK;
                    s->left->color = Color::BLACK;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = Color::BLACK;
    }

    // Внутрішній рекурсивний пошук вузла за ключем
    RBNode* searchTreeHelper(RBNode* node, Rational key)
    {
        if (node == TNULL || key == node->key)
            return node;

        if (key < node->key)
            return searchTreeHelper(node->left, key);
        return searchTreeHelper(node->right, key);
    }

    // Внутрішній метод видалення вузла та каскадного перерахунку сум
    void deleteNodeHelper(RBNode* node, Rational key)
    {
        RBNode* z = TNULL;
        RBNode* x, * y;
        z = searchTreeHelper(node, key);

        if (z == TNULL)
        {
            std::cout << "Element " << key << " not found for deletion.\n";
            return;
        }

        y = z;
        Color y_original_color = y->color;
        RBNode* parent_to_update = nullptr;

        if (z->left == TNULL)
        {
            x = z->right;
            parent_to_update = z->parent;
            rbTransplant(z, z->right);
        }
        else if (z->right == TNULL)
        {
            x = z->left;
            parent_to_update = z->parent;
            rbTransplant(z, z->left);
        }
        else
        {
            y = minimum(z->right);
            y_original_color = y->color;
            x = y->right;

            if (y->parent == z)
            {
                x->parent = y;
                parent_to_update = y;
            }
            else
            {
                parent_to_update = y->parent;
                rbTransplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }

            rbTransplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        // Перераховуємо суми від точки структурних змін вгору до самого кореня
        if (x != TNULL)
        {
            updateSumsUpward(x);
        }
        else
        {
            updateSumsUpward(parent_to_update);
        }

        delete z; // Звільнення динамічної пам'яті

        if (y_original_color == Color::BLACK)
        {
            fixDelete(x);
        }
    }

    // Обчислення суми елементів на проміжку 
    Rational queryRangeHelper(RBNode* node, Rational low, Rational high) const
    {
        if (node == TNULL || node == nullptr) return Rational(0, 1);

        Rational current_sum(0, 1);
        if (node->key >= low && node->key <= high)
        {
            current_sum = current_sum + node->key;
        }

        if (node->left != TNULL)
            current_sum = current_sum + queryRangeHelper(node->left, low, high);

        if (node->right != TNULL)
            current_sum = current_sum + queryRangeHelper(node->right, low, high);

        return current_sum;
    }

    
    void printHelper(RBNode* root, std::string indent, bool last) const
    {
        if (root != TNULL)
        {
            std::cout << indent;
            if (last)
            {
                std::cout << "R----";
                indent += "     ";
            }
            else
            {
                std::cout << "L----";
                indent += "|    ";
            }

            std::string sColor = (root->color == Color::RED) ? "RED" : "BLACK";
            std::cout << root->key << " (" << sColor << ", subtree sum: " << root->sum << ")\n";
            printHelper(root->left, indent, false);
            printHelper(root->right, indent, true);
        }
    }

    // Рекурсивне очищення дерева
    void clear(RBNode* node)
    {
        if (node == TNULL || node == nullptr) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

public:
    SegmentRBTree()
    {
        TNULL = new RBNode(Rational(0, 1));
        TNULL->color = Color::BLACK;
        TNULL->left = nullptr;
        TNULL->right = nullptr;
        root = TNULL;
    }

    ~SegmentRBTree()
    {
        clear(root);
        delete TNULL;
    }

    // Вставка нового елемента
    void insert(Rational k)
    {
        RBNode* node = new RBNode(k);
        node->left = TNULL;
        node->right = TNULL;

        RBNode* y = nullptr;
        RBNode* x = this->root;

        while (x != TNULL)
        {
            y = x;
            if (node->key < x->key)
                x = x->left;
            else
                x = x->right;
        }

        node->parent = y;
        if (y == nullptr)
            root = node;
        else if (node->key < y->key)
            y->left = node;
        else
            y->right = node;

        updateSumsUpward(node);

        if (node->parent == nullptr)
        {
            node->color = Color::BLACK;
            return;
        }

        if (node->parent->parent == nullptr) return;

        fixInsert(node);
        updateSumsUpward(node);
    }

    void deleteNode(Rational key)
    {
        deleteNodeHelper(this->root, key);
    }

    Rational queryRange(Rational low, Rational high) const
    {
        return queryRangeHelper(root, low, high);
    }

    // Друкування структури дерева
    void printTree() const
    {
        if (root == TNULL) std::cout << "Tree is empty.\n";
        else printHelper(this->root, "", true);
    }
};


int main()
{
    BTree<3> btree;
    SegmentRBTree segTree;

    std::cout << "========================================================\n";
    std::cout << " LABORATORY WORK #2: DATA STRUCTURES\n";
    std::cout << "========================================================\n\n";

    std::cout << ">>> Testing B-Tree\n";
    std::cout << "Inserting fractions: 1/2, 3/4, 1/4\n";
    btree.insert(Rational(1, 2));
    btree.insert(Rational(3, 4));
    btree.insert(Rational(1, 4));

    std::cout << "\nB-Tree structure:\n";
    btree.print();

    std::cout << "\n--------------------------------------------------------\n";

    std::cout << ">>> Testing Segment Red-Black Tree\n";
    std::cout << "Inserting elements: 1/2, 1/4, 3/4, 1/8, 5/8\n";

    segTree.insert(Rational(1, 2));
    segTree.insert(Rational(1, 4));
    segTree.insert(Rational(3, 4));
    segTree.insert(Rational(1, 8));
    segTree.insert(Rational(5, 8));

    std::cout << "\nRed-Black Tree BEFORE DELETION:\n";
    segTree.printTree();

    Rational low(1, 4), high(3, 4);
    std::cout << "\nSum on range [" << low << ", " << high << "] before deletion: ";
    std::cout << segTree.queryRange(low, high) << " (Expected: 17/8)\n";

    // Тестування роботи функції видалення вузла
    std::cout << "\nDeleting element 1/2 from the tree...\n";
    segTree.deleteNode(Rational(1, 2));

    std::cout << "\nRed-Black Tree AFTER DELETION of element 1/2:\n";
    segTree.printTree();

    std::cout << "\nSum on range [" << low << ", " << high << "] after deletion of 1/2: ";
    std::cout << segTree.queryRange(low, high) << " (Expected: 1/4 + 5/8 + 3/4 = 13/8)\n";

    std::cout << "\n========================================================\n";
    return 0;
}