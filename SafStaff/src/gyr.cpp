#include "gyr.h"



Gyro::Gyro() {     // Constructor

}

void Gyro::update(float x, float y, float z, float ax, float ay, float az) {
    gyro_t_pt currentGyro{x, y, z, ax, ay, az};
    m_calcs.t_pt = currentGyro;
    gyroHistory.push_front(currentGyro);
    if (gyroHistory.size()>10){gyroHistory.pop_back();}
    updateCalcs();
}

void Gyro::updateCalcs()
{
    float avgXAcc = 0;
    float avgYAcc = 0;
    float avgZAcc = 0;

    float minXAngle = 1e16;
    float minYAngle = 1e16;
    float minZAngle = 1e16;
    float maxXAngle = -1e16;
    float maxYAngle = -1e16;
    float maxZAngle = -1e16;

    m_calcs.maxZAcc = -100;
    m_calcs.minZAcc = 100;

    int loopNum = 0;
    int onlyFirstNum = 5;
    std::list<gyro_t_pt>::iterator it;
    for (it = gyroHistory.begin(); it != gyroHistory.end(); it++)
    {
        //if we onlt care for the n most recent values
        if (loopNum<onlyFirstNum) 
        {
            if (minXAngle > it->angleX){minXAngle = it->angleX;}
            if (maxXAngle < it->angleX){maxXAngle = it->angleX;}            
            if (minYAngle > it->angleY){minYAngle = it->angleY;}
            if (maxYAngle < it->angleY){maxYAngle = it->angleY;}            
            if (minZAngle > it->angleZ){minZAngle = it->angleZ;}
            if (maxZAngle < it->angleZ){maxZAngle = it->angleZ;}
            
            if (m_calcs.maxZAcc < it->accZ){m_calcs.maxZAcc = it->accZ;}
            if (m_calcs.minZAcc > it->accZ){m_calcs.minZAcc = it->accZ;}
        }

        avgXAcc += it->accX;
        avgYAcc += it->accY;
        avgZAcc += it->accZ;
             
        //add lots of other calcs!!!!
    }
    m_calcs.averageXAcc = avgXAcc/(gyroHistory.size());
    m_calcs.averageYAcc = avgYAcc/(gyroHistory.size());
    m_calcs.averageZAcc = avgZAcc/(gyroHistory.size());
    m_calcs.rangeZacc = m_calcs.maxZAcc - m_calcs.minZAcc;

    m_calcs.currentXSpeed = maxXAngle - minXAngle;
    m_calcs.currentYSpeed = maxYAngle - minYAngle;
    m_calcs.currentZSpeed = maxZAngle - minZAngle;
 
    m_calcs.currentXYZSpeed = m_calcs.currentXSpeed + m_calcs.currentYSpeed + m_calcs.currentZSpeed;
    loopNum++;
}

const char* Gyro::display()
{
    std::ostringstream ss;
    ss << gyroHistory.size();
    ss <<    "  ZaccAvg:"<<   m_calcs.averageZAcc;
    ss <<    "  ZaccRange:"<<   m_calcs.rangeZacc;
    ss <<    "  Zspeed:"<<   m_calcs.currentZSpeed;
    ss <<    "  XYZspeed:"<<   m_calcs.currentXYZSpeed;

    std::string str = ss.str();
    const char* s = str.c_str();
    
    return s;
}