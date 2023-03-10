/*
    SPDX-FileCopyrightText: 2010 Jean-Baptiste Mardelle <jb@kdenlive.org>

SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "v4lcapture.h"
#include "kdenlivesettings.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include <sys/ioctl.h>

V4lCaptureHandler::V4lCaptureHandler() = default;

// static

QStringList V4lCaptureHandler::getDeviceName(const QString &input)
{

    char *src = strdup(input.toUtf8().constData());
    QString pixelformatdescription;
    int fd = open(src, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        free(src);
        return QStringList();
    }
    struct v4l2_capability cap
    {
    };

    char *devName = nullptr;
    int captureEnabled = 1;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        fprintf(stderr, "Cannot get capabilities.");
        // return nullptr;
    } else {
        devName = strdup(reinterpret_cast<char *>(cap.card));
        if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0u) {
            // Device cannot capture
            captureEnabled = 0;
        }
    }

    if (captureEnabled != 0) {
        struct v4l2_format format
        {
        };
        memset(&format, 0, sizeof(format));
        format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        struct v4l2_fmtdesc fmt
        {
        };
        memset(&fmt, 0, sizeof(fmt));
        fmt.index = 0;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        struct v4l2_frmsizeenum sizes
        {
        };
        memset(&sizes, 0, sizeof(sizes));

        struct v4l2_frmivalenum rates
        {
        };
        memset(&rates, 0, sizeof(rates));
        char value[200];

        while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) != -1) {
            if (pixelformatdescription.length() > 2000) {
                break;
            }
            if (snprintf(value, sizeof(value), ">%c%c%c%c", fmt.pixelformat >> 0, fmt.pixelformat >> 8, fmt.pixelformat >> 16, fmt.pixelformat >> 24) > 0) {
                pixelformatdescription.append(value);
            }
            fprintf(stderr, "detected format: %s: %c%c%c%c\n", fmt.description, fmt.pixelformat >> 0, fmt.pixelformat >> 8, fmt.pixelformat >> 16,
                    fmt.pixelformat >> 24);

            sizes.pixel_format = fmt.pixelformat;
            sizes.index = 0;
            // Query supported frame size
            while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &sizes) != -1) {
                struct v4l2_frmsize_discrete image_size = sizes.discrete;
                // Query supported frame rates
                rates.index = 0;
                rates.pixel_format = fmt.pixelformat;
                rates.width = image_size.width;
                rates.height = image_size.height;
                if (pixelformatdescription.length() > 2000) {
                    break;
                }
                if (snprintf(value, sizeof(value), ":%dx%d=", image_size.width, image_size.height) > 0) {
                    pixelformatdescription.append(value);
                }
                fprintf(stderr, "Size: %dx%d: ", image_size.width, image_size.height);
                while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &rates) != -1) {
                    if (pixelformatdescription.length() > 2000) {
                        break;
                    }
                    if (snprintf(value, sizeof(value), "%d/%d,", rates.discrete.denominator, rates.discrete.numerator) > 0) {
                        pixelformatdescription.append(value);
                    }
                    fprintf(stderr, "%d/%d, ", rates.discrete.numerator, rates.discrete.denominator);
                    rates.index++;
                }
                fprintf(stderr, "\n");
                sizes.index++;
            }
            fmt.index++;
        }
    }
    close(fd);
    free(src);

    QStringList result;
    if (devName == nullptr) {
        return result;
    }
    QString deviceName(devName);
    free(devName);
    result << (deviceName.isEmpty() ? input : deviceName) << pixelformatdescription;
    return result;
}
