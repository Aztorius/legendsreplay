#include "keyframe.h"

Keyframe::Keyframe()
{
    m_id = 0;
    m_nextchunkid = 0;
    m_data = "";
}

Keyframe::Keyframe(int id, QByteArray data, int nextchunkid)
{
    m_id = id;
    m_nextchunkid = nextchunkid;
    m_data = data;
}

Keyframe::~Keyframe()
{

}

int Keyframe::getId() const
{
    return m_id;
}

int Keyframe::getNextchunkid() const
{
    return m_nextchunkid;
}

QByteArray Keyframe::getData() const
{
    return m_data;
}

int Keyframe::getSize() const
{
    return m_data.size();
}

bool Keyframe::isEmpty() const
{
    return m_data.isEmpty();
}
