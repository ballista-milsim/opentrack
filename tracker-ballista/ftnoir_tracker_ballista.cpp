#include "ftnoir_tracker_ballista.h"
#include "api/plugin-api.hpp"
#include "compat/math.hpp"

ballista::ballista()
{
    if (s.guid->isEmpty())
    {
        std::vector<win32_joy_ctx::joy_info> info = joy_ctx.get_joy_info();
        if (!info.empty())
        {
            s.guid = info[0].guid;
            s.b->save();
        }
    }
    if (s.com_port_name->isEmpty())
    {
        for (const QSerialPortInfo& port_info : QSerialPortInfo::availablePorts())
        {
            s.com_port_name = port_info.portName();
            s.b->save();
            break;
        }
    }
}

ballista::~ballista() = default;

void ballista::data(double *data)
{
    int map[6] = {
        s.joy_1 - 1, s.joy_2 - 1, s.joy_3 - 1, s.joy_4 - 1, s.joy_5 - 1, s.joy_6 - 1,
    };

    bool map_abs[6] = {
        s.joy_1_abs, s.joy_2_abs, s.joy_3_abs, s.joy_4_abs, s.joy_5_abs, s.joy_6_abs,
    };

    const double limits[] = { 100, 100, 100, 180, 180, 180 };

    const QString guid = s.guid;
    int axes[8];
    const bool ret = joy_ctx.poll_axis(guid, axes);

    if (ret)
    {
        for (int i = 0; i < 6; i++)
        {
            int k = map[i] - 1;
            if (k < 0 || k >= 8)
                data[i] = 0;
            else
                data[i] = clamp(axes[k] * limits[i] / AXIS_MAX, -limits[i], limits[i]);
            if (map_abs[i] == true)
                data[i] = abs(data[i]);
        }
    }

    if (t.head_pose) {
        tobii_head_pose_t p = *t.head_pose;
        if (p.position_validity == TOBII_VALIDITY_VALID) {
            if (center_pose.position_validity == TOBII_VALIDITY_VALID) {
                p.position_xyz[0] = p.position_xyz[0] - center_pose.position_xyz[0];
                p.position_xyz[1] = p.position_xyz[1] - center_pose.position_xyz[1];
                p.position_xyz[2] = p.position_xyz[2] - center_pose.position_xyz[2];
            }
            else {
                center_pose = p;
            }
            //data[0] = clamp(p.position_xyz[0] * 30.0 / 300.0, -30.0, 30.0);
            //data[1] = clamp(p.position_xyz[1] * 30.0 / 300.0, -30.0, 30.0);
            data[2] = clamp(p.position_xyz[2] * 30.0 / 300.0, -30.0, 30.0);
        }

        return;

        double max_yaw = 90.0;
        if (p.rotation_validity_xyz[1] == TOBII_VALIDITY_VALID) {
            if (center_pose.rotation_validity_xyz[1] == TOBII_VALIDITY_VALID) {
                p.rotation_xyz[1] = p.rotation_xyz[1] - center_pose.rotation_xyz[1];
            }
            data[3] = clamp(p.rotation_xyz[1] * 100.0 * max_yaw / 90.0, -max_yaw, max_yaw);
        }

        double max_pitch = 90.0;
        if (p.rotation_validity_xyz[0] == TOBII_VALIDITY_VALID) {
            if (center_pose.rotation_validity_xyz[0] == TOBII_VALIDITY_VALID) {
                p.rotation_xyz[0] = p.rotation_xyz[0] - center_pose.rotation_xyz[0];
            }
            data[4] = clamp(p.rotation_xyz[0] * 100.0 * max_pitch / 90.0, -max_pitch, max_pitch);
        }

        double max_roll = 90.0;
        if (p.rotation_validity_xyz[2] == TOBII_VALIDITY_VALID) {
            if (center_pose.rotation_validity_xyz[2] == TOBII_VALIDITY_VALID) {
                p.rotation_xyz[2] = p.rotation_xyz[2] - center_pose.rotation_xyz[2];
            }
            data[5] = clamp(p.rotation_xyz[2] * 100.0 * max_roll / 90.0, -max_roll, max_roll);
        }
    }
}

module_status ballista::start_tracker(QFrame*)
{
    t.start();
    calibrate();
    return status_ok();
}
bool ballista::center()
{
    if (t.head_pose)
    {
        center_pose = *t.head_pose;
    }
    reset();
    return true;
}

void ballista::reset()
{
    const QString com_port_name = s.com_port_name;
    QSerialPort com_port;
    com_port.setPortName(com_port_name);
    if (com_port.open(QIODevice::ReadWrite))
    {
        com_port.setBaudRate(QSerialPort::Baud9600);
        com_port.setDataBits(QSerialPort::Data8);
        com_port.setParity(QSerialPort::NoParity);
        com_port.setStopBits(QSerialPort::OneStop);
        com_port.setFlowControl(QSerialPort::NoFlowControl);
        com_port.clear(QSerialPort::AllDirections);
        QByteArray cmd("R");
        com_port.write(cmd);
        com_port.waitForBytesWritten(1000);
        com_port.close();
    }
}

void ballista::calibrate()
{
    const QString com_port_name = s.com_port_name;
    QSerialPort com_port;
    com_port.setPortName(com_port_name);
    if (com_port.open(QIODevice::ReadWrite))
    {
        com_port.setBaudRate(QSerialPort::Baud9600);
        com_port.setDataBits(QSerialPort::Data8);
        com_port.setParity(QSerialPort::NoParity);
        com_port.setStopBits(QSerialPort::OneStop);
        com_port.setFlowControl(QSerialPort::NoFlowControl);
        com_port.clear(QSerialPort::AllDirections);
        QByteArray cmd("r");
        com_port.write(cmd);
        com_port.waitForBytesWritten(1000);
        com_port.close();
    }
}

OPENTRACK_DECLARE_TRACKER(ballista, dialog_ballista, ballistaDll)
