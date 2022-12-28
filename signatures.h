#pragma once
#include <string>
#include <set>
#include <iostream>
#include <QByteArray>
#include <vector>

using std::string, std::set, std::vector;

struct Signature {
    string name;
    short len;
    vector<uchar> seq;
    set<int> any;

    Signature (QByteArray data)
    {
        name = "";
        bool flag = true;
        for (int i = 1; i < data.size(); ++i) {
            if (data[i] == '\r' || data[i] == '\n' || data[i] == ']') break;
            if (data[i] == '=') {
                flag = false;
                len = 0;
                seq = vector<uchar> ((data.size() - i - 4) / 2);
                continue;
            }
            if (flag) {
                name += data[i];
            } else {
                if (data[i] == ':') {
                    any.insert (len++);
                    ++i;
                } else {
                    seq[len++] = ((data[i] >= 'A') ? (data[i] - 'A' + 10) : (data[i] - '0'))
                                 * 16 + ((data[i + 1] >= 'A') ? (data[i + 1] - 'A' + 10) : (data[i + 1] - '0'));
                    ++i;
                }
            }
        }
    }
    ~Signature()
    {
        //        delete[] seq;
    }

    bool compare (uchar *file_data) const
    {
        for (int i = 0; i < len; ++i) {
            if (any.count (i)) continue;
            if (file_data[i] != seq[i]) return false;
        }
        return true;
    }
};

