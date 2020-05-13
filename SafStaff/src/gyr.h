#include <list>
#include <sstream>

struct gyro_t_pt
{
    float angleX;
    float angleY;
    float angleZ;
    float accX;
    float accY;
    float accZ;
};

struct gyro_calcs
{
    //avg values over history buffer
    float averageXAcc;
    float averageYAcc;
    float averageZAcc;

    float currentXSpeed;
    float currentYSpeed;
    float currentZSpeed;
    float currentXYZSpeed;
    
    float maxZAcc;
    float minZAcc;
    float rangeZacc;

    gyro_t_pt t_pt;
};

class Gyro {  

    public: 
        Gyro(); 
        void update(float x, float y, float z, float ax, float ay, float az);
        gyro_calcs getCalcs() {return m_calcs;}
        gyro_t_pt getTimePt() {return m_calcs.t_pt;}
        const char* display();

    private:
        std::list<gyro_t_pt> gyroHistory;
        gyro_calcs m_calcs;
        void updateCalcs();

};
