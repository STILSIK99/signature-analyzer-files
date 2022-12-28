#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>
#include <QDebug>

#include <iostream>
#include <set>
#include <list>
#include <mutex>
#include <thread>
#include <chrono>

#include "signatures.h"

using namespace std;

typedef unsigned char u_char;

bool readFiles (list<QString> &files, QString path)
{
    if (!QFileInfo (path).isDir()) {
        return false;
    }
    QDirIterator it (path, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo info (it.next());
        files.emplace_back (info.filePath());
    }
    return true;
}

bool loadSigns (list<Signature> &signs, QString path)
{
    QFile file (path);
    file.open (QIODevice::ReadOnly);
    if (!file.isOpen()) {
        return false;
    }
    while (!file.atEnd()) {
        QByteArray readed = file.readLine();
        if (readed.size() < 3) continue;
        signs.emplace_back (Signature (readed));
    }
    file.close();
}

string getString()
{
    string s = "";
    do {
        getline (cin, s);
    } while (s == "");
    return s;
}

mutex _lpost;
mutex _lprev;

map<QString, const Signature *> result;

list<Signature> signs;
list<QString> queue;

void process ()
{
    short MAX_LEN = 0;
    for (auto &x : signs) {
        MAX_LEN = max (MAX_LEN, x.len);
    }
    while (true) {
        _lprev.lock();
        if (queue.size() == 0) {
            _lprev.unlock();
            break;
        }
        cout << "Remaining " << queue.size() << "\n";
        QString next_file = queue.front();
        queue.pop_front();
        _lprev.unlock();

        QFile file (next_file);
        file.open (QIODevice::ReadOnly);
        if (!file.isOpen()) {
            cerr << "File not opened.\n";
            continue;
        }
        int readed = min ((int)MAX_LEN, (int)file.size());
        uchar *data = new uchar[MAX_LEN];
        file.read (reinterpret_cast<char *> (data), MAX_LEN);
        file.close();
        for (auto &sign : signs) {
            if (readed < sign.len) continue;
            if (sign.compare (data)) {
                _lpost.lock();
                result[next_file] = &sign;
                _lpost.unlock();
                break;
            }
        }
    }
}


int main()
{
    //    list<Signature> signs;
    cout << "Input file with signatures:\n";
    if (!loadSigns (signs, QString::fromStdString (getString()))) {
        cerr << "Signatures not load!\n";
        return 0;
    } else {
        cout << "Signatures was loaded!\n";
    }
    //    list<QString> files;
    cout << "Input path to directory:\n";
    if (!readFiles (queue, QString::fromStdString (getString()))) {
        cerr << "Directory not found!\n";
        return 0;
    } else {
        cout << queue.size() << " files were found!\n";
    }

    cout << "__________________________________________________________\n";
    cout << "                                                          \n";
    cout << "                        SEARCHING:                        \n";
    cout << "                                                          \n";
    //    process();
    thread call (process);
    thread call2 (process);
    thread call3 (process);
    thread call4 (process);
    thread call5 (process);

    call.join();
    call2.join();
    call3.join();
    call4.join();
    call5.join();
    cout << "__________________________________________________________\n";
    cout << "                                                          \n";
    cout << "                         RESULTS:                         \n";
    cout << "                                                          \n";

    cout << result.size() << " signatures were found.\n";
    for (auto &x : result) {
        cout << x.first.toStdString() << ": " << x.second->name;
        cout << "\n";
    }
    return 0;
}
