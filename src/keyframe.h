#ifndef KEYFRAME_H
#define KEYFRAME_H

#include <QByteArray>

class Keyframe
{
public:
    Keyframe();
    Keyframe(int id, QByteArray data, int nextchunkid);
    ~Keyframe();
    int getId() const;
    int getNextchunkid() const;
    QByteArray getData() const;
    int getSize() const;

private:
    int m_id;
    int m_nextchunkid;
    QByteArray m_data;
};

#endif // KEYFRAME_H
