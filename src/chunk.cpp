#include "chunk.h"

Chunk::Chunk()
{
    m_id = 0;
    m_keyframeid = 0;
    m_data = "";
    m_duration = 30000;
}

Chunk::Chunk(int id, QByteArray data, int keyframeid, int duration)
{
    m_id = id;
    m_keyframeid = keyframeid;
    m_data = data;
    m_duration = duration;
}

Chunk::~Chunk()
{

}

int Chunk::getId() const
{
    return m_id;
}

int Chunk::getKeyframeId() const
{
    return m_keyframeid;
}

QByteArray Chunk::getData() const
{
    return m_data;
}

int Chunk::getDuration() const
{
    return m_duration;
}

int Chunk::getSize() const
{
    return m_data.size();
}
