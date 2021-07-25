#include "TTree.hpp"
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <cassert>
using namespace NStd;

//k = (y1-y2)/(x1-x2)
//b = (x1*x2 + y1*y2)/(x1-x2)
int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    assert(argc > 1);
    if(strcmp(argv[1], "index") == 0) {
        std::string input, output;
        for(int i = 2; i < argc; ++i){
            if(strcmp(argv[i], "--input") == 0){
                input = argv[i+1];
            }
            else if(strcmp(argv[i], "--output") == 0){
                output = argv[i+1];
            }
        }
        if(input.empty() || output.empty()){
            std::cout << "Not all input files!\n";
            return -1;
        }
        std::ifstream ifs;
        ifs.open(input.c_str());
        int n;
        double x, y;
        TPoint first, second;
        std::vector<TSegment> segments;
        std::map<double, std::vector<TLine>> lines;
        std::map<double, std::vector<TLine>> ends;
        int index = 0;
        TTree tree;
        std::vector<double> rawIntvs;

        while (ifs >> n) {
            TPoint start;
            for (int i = 0; i < n; ++i) {
                ifs >> x >> y;
                rawIntvs.push_back(x);
                if (i == 0) {
                    start = TPoint(x, y);
                    first = start;
                }
                if (i == n - 1) {
                    second = TPoint(x, y);
                    if (start.x != second.x) {
                        TLine buf(start, second, index);
                        lines[buf.xStart].push_back(buf);
                        ends[buf.xEnd].push_back(buf);
                    }
                    if (first.x != second.x) {
                        TLine buf(first, second, index);
                        lines[buf.xStart].push_back(buf);
                        ends[buf.xEnd].push_back(buf);
                    }
                } else if (i > 0) {
                    second = TPoint(x, y);
                    if (first.x != second.x) {
                        TLine buf(first, second, index);
                        lines[buf.xStart].push_back(buf);
                        ends[buf.xEnd].push_back(buf);
                    }
                    std::swap(first, second);
                }
            }
            ++index;
        }
        std::vector<double> intvs;
        DeleteDuplicates(rawIntvs, intvs);
        int segmentIndex = 0;
        intvs.push_back(intvs[intvs.size() - 1]);
        auto it = lines.begin();
        auto it2 = ends.begin();
        int version = -1;
        while (it != lines.end() || it2 != ends.end()) {
            if (it == lines.end()) {
                for (auto &bufIt : it2->second) {
                    tree.Delete(TKey(bufIt));
                    ++version;
                }
                segments.emplace_back(version, intvs[segmentIndex], intvs[segmentIndex + 1]);
                ++segmentIndex;
                ++it2;
            } else if (it->first == it2->first) {
                for (auto &bufIt : it2->second) {
                    tree.Delete(TKey(bufIt));
                    ++version;
                }
                for (auto &bufIt : it->second) {
                    tree.Insert(TKey(bufIt));
                    ++version;
                }
                segments.emplace_back(version, intvs[segmentIndex], intvs[segmentIndex + 1]);
                ++segmentIndex;
                ++it;
                ++it2;
            } else if (it->first < it2->first) {
                for (auto &bufIt : it->second) {
                    tree.Insert(TKey(bufIt));
                    ++version;
                }
                segments.emplace_back(version, intvs[segmentIndex], intvs[segmentIndex + 1]);
                ++segmentIndex;
                ++it;
            } else if (it2->first < it->first) {
                for (auto &bufIt : it2->second) {
                    tree.Delete(TKey(bufIt));
                    ++version;
                }
                segments.emplace_back(version, intvs[segmentIndex], intvs[segmentIndex + 1]);
                ++segmentIndex;
                ++it2;
            }
        }
        tree.Save(output, segments);
    }
    else if(strcmp(argv[1], "search") == 0) {
        std::string input, output, index;

        for(int i = 2; i < argc; ++i){
            if(strcmp(argv[i], "--index") == 0){
                index = argv[i+1];
            }
            else if(strcmp(argv[i], "--output") == 0){
                output = argv[i+1];
            }
            else if(strcmp(argv[i], "--input") == 0){
                input = argv[i+1];
            }
        }
        std::shared_ptr<std::ifstream> ifs = std::make_shared<std::ifstream>(std::ifstream(input));
        std::shared_ptr<std::ofstream> ofs = std::make_shared<std::ofstream>(std::ofstream(output));
        TTree tree;
        std::vector<TSegment> segments;
        tree.Load(index, segments);
        TPoint point;
        double x, y;
        int size = (int)segments.size();
        while(*ifs >> x >> y) {
            point = TPoint(x, y);
            tree.Search(point, binSearch(segments, x), size, ofs);
        }
    }
}
