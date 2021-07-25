#ifndef DALAB2_TAVLTREE_H
#define DALAB2_TAVLTREE_H

#include <algorithm>
#include<cstdlib>
#include<cstring>
#include<cstdio>
#include<iostream>
#include<fstream>
#include<new>
#include<vector>
#include <memory>
#include <set>
#include <string>
namespace NStd {
    const short MAX_KEY_LENGTH = 256;

    void DeleteDuplicates(std::vector<double> &a, std::vector<double> &out){
        std::set <double> s;
        int size = (int)a.size();
        for (int i = 0; i < size; ++i) {
            if (s.count(a[i]) == 0) {
                out.push_back(a[i]);
                s.insert(a[i]);
            }
        }
        std::sort(out.begin(), out.end());
    }
    struct TSegment{
        int version;
        double x1, x2;
        TSegment(int _version, double _x1, double _x2) : version(_version), x1 (_x1), x2(_x2){}
        TSegment() = default;
    };

    bool operator > (const TSegment &lhs, double &rhs){
        return lhs.x1 > rhs;
    }

    bool operator == (const TSegment &lhs, double &rhs) {
        return (lhs.x1 <= rhs && lhs.x2 >= rhs);
    }

    int binSearch(std::vector<TSegment> segments, double x){
        int l = 0;
        int r = (int)segments.size() - 1;
        int mid = (l + r)/2;
        bool flag = false;
        while ((l <= r) && !flag) {
            mid = (l + r) / 2; // считываем срединный индекс отрезка [l,r]

            if (segments[mid] == x) {
                flag = true; //проверяем ключ со серединным элементом
            }
            if (segments[mid] > x) {
                r = mid - 1; // проверяем, какую часть нужно отбросить
            }
            else l = mid + 1;
        }
        return segments[mid].version;
    }

    struct TPoint{
        double x, y;
        TPoint(double _x, double _y): x(_x), y(_y){}
        TPoint() = default;
    };

    // y1 = kx1 + b
    // y2 = kx2 + b
    // k
    // b = y1 - kx1
    struct TLine{
        double k, b;
        int index = -1;
        double xStart;
        double xEnd;
        TLine(TPoint first, TPoint second, int _index) : index(_index){
            if(first.x > second.x){
                std::swap(first, second);
            }
            k = (first.y - second.y) / (first.x - second.x);
            //k = (y1-y2)/(x1-x2);
            b = first.y - k*first.x;
            xStart = std::min(first.x, second.x);
            xEnd = std::max(first.x, second.x);
        }
        TLine() = default;
        double getY(double x) const{
            return k*x + b;
        }
    };

    bool operator < (const TLine &lhs, const TLine &rhs){
        if(lhs.xStart != rhs.xStart)
            return lhs.getY(std::max(lhs.xStart, rhs.xStart)) < rhs.getY(std::max(lhs.xStart, rhs.xStart));
        else if(lhs.k != rhs.k && lhs.getY(lhs.xStart) == rhs.getY(rhs.xStart))
            return lhs.k < rhs.k;
        else
            return lhs.b < rhs.b;
    }

    bool operator > (const TLine &lhs, const TLine &rhs){
        if(lhs.xStart != rhs.xStart)
            return lhs.getY(std::max(lhs.xStart, rhs.xStart)) > rhs.getY(std::max(lhs.xStart, rhs.xStart));
        else if(lhs.k != rhs.k && lhs.getY(lhs.xStart) == rhs.getY(rhs.xStart))
            return lhs.k > rhs.k;
        else
            return lhs.b > rhs.b;
    }

    bool operator == (const TLine &lhs, const TLine &rhs){
        if(lhs.k == rhs.k && lhs.b == rhs.b && lhs.index == rhs.index && lhs.xStart == rhs.xStart && lhs.xEnd == rhs.xEnd)
            return true;
        else
            return false;
    }

    bool operator < (const TLine &lhs, const TPoint &rhs){
        return lhs.getY(rhs.x) < rhs.y;
    }
    bool operator > (const TLine &lhs, const TPoint &rhs){
        return lhs.getY(rhs.x) > rhs.y;
    }

    struct TKey {
        TLine line;

        explicit TKey(TLine _line) : line(_line) {}

        TKey() = default;
    };

    bool operator<(const TKey &lhs, const TKey &rhs) {
        return lhs.line < rhs.line;
    }

    bool operator>(const TKey &lhs, const TKey &rhs) {
        return lhs.line > rhs.line;
    }

    bool operator==(const TKey &lhs, const TKey &rhs) {
        return lhs.line == rhs.line;
    }

    class TNode {
    public:
        TKey key = TKey();
        short balance = 0;
        std::shared_ptr<TNode> left;
        std::shared_ptr<TNode> right;

        int version = 0;
        TNode() : key() {}

        TNode(TKey _key, short _balance, int _version) : key(_key), balance(_balance), version(_version) {}

        TNode(TKey _key, int _version) : key(_key), version(_version) {}
    };


    class TTree {
    public:
        TTree() {}

        void Insert(TKey key);

        void Delete(TKey key);

        void Search(TPoint key, int version, int count, std::shared_ptr<std::ofstream> &ofs);

        int Count(int version);

        void Save(std::string &path, std::vector<TSegment> &indexes);

        void Load(std::string &path, std::vector<TSegment> &indexes);

        ~TTree() {
            int size = (int)roots.size();
            for(int i = size - 1; i >= 0; --i)
                DeleteTree(roots[i]);
        }

    private:
        std::shared_ptr<TNode>RotateLeft(std::shared_ptr<TNode> node);

        std::shared_ptr<TNode>RotateRight(std::shared_ptr<TNode> node);

        std::shared_ptr<TNode>Balance(std::shared_ptr<TNode> node);

        std::shared_ptr<TNode>BigLeftRotation(std::shared_ptr<TNode> node);

        std::shared_ptr<TNode>BigRightRotation(std::shared_ptr<TNode> node);

        std::shared_ptr<TNode>DeleteHelper(std::shared_ptr<TNode> node, std::shared_ptr<TNode> rNode);

        std::shared_ptr<TNode>Insert(TKey key, std::shared_ptr<TNode> node);

        std::shared_ptr<TNode>Delete(TKey key, std::shared_ptr<TNode> node);

        std::shared_ptr<TNode>Search(TPoint key, std::shared_ptr<TNode> node);

        void SearchHelper(std::shared_ptr<TNode>node, std::vector<int> &ans, TPoint &key);

        void DeleteTree(std::shared_ptr<TNode> node);

        void SaveHelper(std::shared_ptr<TNode> &node, std::ofstream &of);

        std::shared_ptr<TNode> LoadHelper(std::ifstream &ifs, int count, int &curCount);

        std::vector<std::shared_ptr<TNode>> roots;

        void CountHelper (std::shared_ptr<TNode> node, int &res);

        int curVersion = 0;
    };

    bool stopBalancing;

    std::shared_ptr<TNode> TTree::RotateLeft(std::shared_ptr<TNode> node) {
        std::shared_ptr<TNode> b = node->right;
        if(b != nullptr && b->version != curVersion){
            b = std::make_shared<TNode>(*b);
            b->version = curVersion;
        }
        if(node->version != curVersion){
            node = std::make_shared<TNode>(*node);
            node->version = curVersion;
        }
        if (node == roots[curVersion]) {
            roots[curVersion] = b;
        }
        node->right = b->left;
        b->left = node;

        return b;
    }

    std::shared_ptr<TNode>TTree::RotateRight(std::shared_ptr<TNode>node) {
        std::shared_ptr<TNode>b = node->left;
        if(b != nullptr && b->version != curVersion){
            b = std::make_shared<TNode>(*b);
            b->version = curVersion;
        }
        if(node->version != curVersion){
            node = std::make_shared<TNode>(*node);
            node->version = curVersion;
        }
        if (node == roots[curVersion]) {
            roots[curVersion] = b;
        }
        node->left = b->right;
        b->right = node;
        return b;
    }

    std::shared_ptr<TNode>TTree::BigLeftRotation(std::shared_ptr<TNode>node) {
        node->right = RotateRight(node->right);
        node = RotateLeft(node);
        if (node->balance == 1) {
            node->left->balance = 0;
            node->right->balance = -1;
            node->balance = 0;
        } else if (node->balance == -1) {
            node->left->balance = 1;
            node->right->balance = 0;
            node->balance = 0;
        } else if (node->balance == 0) {
            node->left->balance = 0;
            node->right->balance = 0;
        }
        return node;
    }

    std::shared_ptr<TNode>TTree::BigRightRotation(std::shared_ptr<TNode>node) {
        node->left = RotateLeft(node->left);
        node = RotateRight(node);
        if (node->balance == 1) {
            node->left->balance = 0;
            node->right->balance = -1;
            node->balance = 0;
        } else if (node->balance == -1) {
            node->left->balance = 1;
            node->right->balance = 0;
            node->balance = 0;
        } else if (node->balance == 0) {
            node->left->balance = 0;
            node->right->balance = 0;
        }
        return node;
    }

    std::shared_ptr<TNode>TTree::Balance(std::shared_ptr<TNode>node) {
        if (node == nullptr) { return nullptr; }
        if (node->balance == 2) {
            if (node->left->balance != -1) {
                node = RotateRight(node);
                if (node->balance == 1) {
                    node->right->balance = 0;
                    node->balance = 0;
                } else if (node->balance == 0) {
                    node->right->balance = 1;
                    node->balance = -1;
                }
            } else {
                node = BigRightRotation(node);
            }
        } else if (node->balance == -2) {
            if (node->right->balance != 1) {
                node = RotateLeft(node);
                if (node->balance == -1) {
                    node->left->balance = 0;
                    node->balance = 0;
                } else if (node->balance == 0) {
                    node->left->balance = -1;
                    node->balance = 1;
                }
            } else {
                node = BigLeftRotation(node);
            }
        }
        return node;
    }

    void TTree::Insert(TKey key) {
        if (roots.empty() || roots[curVersion - 1] == nullptr) {
            try {
                roots.push_back(std::make_shared<TNode>(TNode(key, curVersion)));
                ++curVersion;
                
            }
            catch (std::bad_alloc &e) {
                std::cout << "ERROR: No memory\n";
                exit(-1);
            }
            return;
        }

        stopBalancing = false;
        roots.push_back(std::make_shared<TNode>(*roots[curVersion - 1]));
        roots[curVersion] = Insert(key, roots[curVersion]);
        ++curVersion;
    }

    std::shared_ptr<TNode>TTree::Insert(TKey key, std::shared_ptr<TNode>node) {
        if (node == nullptr) {
            node = std::make_shared<TNode>(TNode(key, curVersion));
            
            return node;
        }
        if (node->key > key) {
            if(node->left != nullptr) {
                node->left = std::make_shared<TNode>(*node->left);
                node->left->version = curVersion;
            }
            node->left = Insert(key, node->left);
            if (!stopBalancing) {
                node->balance += 1;
                if (node->balance == 0) {
                    stopBalancing = true;
                }
            }
        } else if (node->key < key) {
            if(node->right != nullptr) {
                node->right = std::make_shared<TNode>(*node->right);
                node->right->version = curVersion;
            }
            node->right = Insert(key, node->right);
            if (!stopBalancing) {
                node->balance -= 1;
                if (node->balance == 0) {
                    stopBalancing = true;
                }
            }
        } else {
            stopBalancing = true;
            std::cout << "Exist\n";
            return node;
        }
        if (node->balance == 2 || node->balance == -2) {
            node = Balance(node);
            if (node->balance == 0) {
                stopBalancing = true;
            }
        }
        return node;
    }

    std::shared_ptr<TNode>TTree::DeleteHelper(std::shared_ptr<TNode>node, std::shared_ptr<TNode>rNode) {
        if (rNode->left != nullptr) {
            rNode->left = std::make_shared<TNode>(*rNode->left);
            rNode->left->version = curVersion;
            rNode->left = DeleteHelper(node, rNode->left);
            if (!stopBalancing) {
                rNode->balance -= 1;
                if (rNode->balance == -1) { stopBalancing = true; }
            }
        } else {
            std::shared_ptr<TNode>rightChild = rNode->right;
            if(rightChild != nullptr) {
                rightChild->version = curVersion;
            }
            node->key = rNode->key;
            rNode.reset();
            rNode = rightChild;
        }
        if (rNode == nullptr) {
            return rNode;
        }
        if (rNode->balance == 2 || rNode->balance == -2) {
            rNode = Balance(rNode);
            if (rNode->balance != 0) {
                stopBalancing = true;
            }
        }
        return rNode;
    }

    void TTree::Delete(TKey key) {
        stopBalancing = false;
        if (roots[curVersion - 1] == nullptr) {
            return;
        }
        roots.push_back(std::make_shared<TNode>(*roots[curVersion - 1]));
        roots[curVersion] = Delete(key, roots[curVersion]);
        ++curVersion;
    }

    std::shared_ptr<TNode>TTree::Delete(TKey key, std::shared_ptr<TNode>node) {
        if (node == nullptr) {
            stopBalancing = true;
            return nullptr;
        }
        if (node->key < key) {
            if(node->right != nullptr) {
                node->right = std::make_shared<TNode>(*node->right);
                node->right->version = curVersion;
            }
            node->right = Delete(key, node->right);
            if (!stopBalancing) {
                node->balance += 1;
                if (node->balance == 1) { stopBalancing = true; }
            }
        } else if (node->key > key) {
            if(node->left != nullptr) {
                node->left = std::make_shared<TNode>(*node->left);
                node->left->version = curVersion;
            }
            node->left = Delete(key, node->left);
            if (!stopBalancing) {
                node->balance -= 1;
                if (node->balance == -1) { stopBalancing = true; }
            }
        } else {
            std::shared_ptr<TNode>leftChild;
            if(node->left != nullptr) {
                leftChild = std::make_shared<TNode>(*node->left);
                leftChild->version = curVersion;
            }
            else{
                leftChild = node->left;
            }
            std::shared_ptr<TNode> rightChild;
            if(node->right != nullptr) {
                rightChild = std::make_shared<TNode>(*node->right);
                rightChild->version = curVersion;
            }
            else{
                rightChild = node->right;
            }
            if (leftChild == nullptr && rightChild == nullptr) {
                node.reset();
                
                return nullptr;
            }
            if (rightChild == nullptr) {
                node.reset();
                
                return leftChild;
            }
            if (leftChild == nullptr) {
                node.reset();
                
                return rightChild;
            }
            node->right = std::make_shared<TNode>(*node->right);
            node->right->version = curVersion;
            node->right = DeleteHelper(node, rightChild);
            if (!stopBalancing) {
                node->balance += 1;
                if (node->balance == 1) { stopBalancing = true; }
            }
            
        }
        if (node->balance == 2 || node->balance == -2) {
            node = Balance(node);
            if (node->balance != 0) {
                stopBalancing = true;
            }
        }
        return node;
    }

    void TTree::Search(TPoint key, int version, int count, std::shared_ptr<std::ofstream> &ofs) {
        std::vector<int>ans(count, 0);
        int res = -1;
        std::shared_ptr<TNode>result = Search(key, roots[version]);
        SearchHelper(result, ans, key);
        for(int i = 0; i < count; ++i){
            if(ans[i] % 2 != 0){
                res = i + 1;
                break;
            }
        }
        if (result != nullptr) {
            *ofs << res << "\n";
        } else {
            *ofs << "-1\n";
        }
    }

    std::shared_ptr<TNode>TTree::Search(TPoint key, std::shared_ptr<TNode>node) {
        if (node == nullptr) {
            return nullptr;
        }
        if (node->key.line < key) {
            return Search(key, node->right);
        } else {
            return node;
        }
    }

    void TTree::SearchHelper(std::shared_ptr<TNode>node, std::vector<int> &ans, TPoint &key) {
        if(node == nullptr){
            return;
        }
        SearchHelper(node->left, ans, key);
        SearchHelper(node->right, ans, key);
        if(node->key.line > key && (node->key.line.xStart <= key.x && key.x <= node->key.line.xEnd))
            ++ans[node->key.line.index];
    }
    void TTree::DeleteTree(std::shared_ptr<TNode>node) {
        if (node != nullptr) {
            DeleteTree(node->left);
            DeleteTree(node->right);
            node.reset();
        }
    }

    bool stopSaving = false;

    void TTree::Save(std::string &path, std::vector<TSegment> &indexes) {

        auto of = std::ofstream(path);
        int size = (int)indexes.size();
        int version = 0;
        of << size << ' ';
        for(auto &it : indexes){
            of << version << ' ' << it.x1 << ' ' << it.x2 << ' ';
            ++version;
        }
        for(int i = 0; i < size; ++i){
            of << Count(indexes[i].version) << ' ';
            SaveHelper(roots[indexes[i].version], of);
        }
        of.close();
    }

    void TTree::SaveHelper(std::shared_ptr<TNode> &node, std::ofstream &of) {
        if(stopSaving) { return; }
        short keyLength;
        if (node == nullptr) {
            keyLength = -1;
            of << keyLength << ' ';
            return;
        }
        /*
         * double k, b;
            int index = -1;
            double xStart;
            double xEnd;*/
        keyLength = 1;
        of << keyLength << ' ';
        of << node->key.line.k  << ' ' << node->key.line.b << ' ' << node->key.line.index << ' ' << node->key.line.xStart
                << ' ' << node->key.line.xEnd << ' ';
        char nodeBalance;
        if (node->balance == -1) {
            nodeBalance = '0';
        }
        if (node->balance == 0) {
            nodeBalance = '1';
        }
        if (node->balance == 1) {
            nodeBalance = '2';
        }
        of << nodeBalance << ' ';
        SaveHelper(node->left, of);
        SaveHelper(node->right, of);
    }

    bool stopLoading = false;

    void TTree::Load(std::string &path, std::vector<TSegment> &indexes) {
        stopLoading = false;
        std::ifstream ifs(path);
        int size;
        ifs >> size;
        int version, lastVersion = 0;
        double x1, x2;
        for(int i = 0; i < size; ++i){
            ifs >> version >> x1 >> x2;
            indexes.emplace_back(version, x1, x2);
            ++lastVersion;
        }
        int sizeR = (int)roots.size();
        for(int i = sizeR - 1; i >= 0; --i)
            DeleteTree(roots[i]);
        roots.clear();
        for(int i = 0; i < lastVersion; ++i){
            int count;
            ifs >> count;
            int curCount = 0;
            auto newRoot = LoadHelper(ifs, count, curCount);
            roots.push_back(newRoot);
        }
        ifs.close();
    }

    bool stopTree = false;
    std::shared_ptr<TNode> TTree::LoadHelper(std::ifstream &ifs, int count, int &curCount) {
        if(stopLoading) {
            return nullptr;
        }
        std::shared_ptr<TNode> node = std::make_shared<TNode>(TNode());
        short keyLength = 0;
        ifs >> keyLength;
        if (keyLength == -1) {
            return nullptr;
        }
        if(curCount == count){
            return nullptr;
        }
        ifs >> node->key.line.k >> node->key.line.b >> node->key.line.index >> node->key.line.xStart>>
            node->key.line.xEnd;
        ++curCount;
        char nodeBalance;
        ifs >> nodeBalance;
        int balance = nodeBalance - '0';
        node->balance = (short)balance;
        if (nodeBalance > '2' || nodeBalance < '0') {
            std::cerr << "ERROR: Wrong file format\n";
            stopLoading = true;
            return nullptr;
        }
        node->left = LoadHelper(ifs, count, curCount);
        node->right = LoadHelper(ifs, count, curCount);
        return node;
    }

    void TTree::CountHelper(std::shared_ptr<TNode> node, int &res) {
        if(node != nullptr){
            ++res;
            CountHelper(node->left, res);
            CountHelper(node->right, res);
        }
    }

    int TTree::Count(int version) {
        int answer = 0;
        CountHelper(roots[version], answer);
        return answer;
    }
}


#endif
