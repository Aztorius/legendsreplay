#ifndef CHUNK_H
#define CHUNK_H

#include <QByteArray>

class Chunk
{
public:
    Chunk();
    Chunk(int id, QByteArray data, int keyframeid, int duration = 30000);
    ~Chunk();
    int getId() const;
    int getKeyframeId() const;
    QByteArray getData() const;
    int getDuration() const;
    int getSize() const;

private:
    int m_id;
    int m_keyframeid;
    QByteArray m_data;
    int m_duration;
};

#endif // CHUNK_H
