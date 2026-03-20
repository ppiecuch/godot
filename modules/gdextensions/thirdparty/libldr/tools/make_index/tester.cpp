// tester.cpp

#include <iostream>
#include <stdexcept>
#include <string>

#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include <QElapsedTimer>

#include <libldr/reader.h>
#include "line_info.inl"

class Task : public QObject
{
    Q_OBJECT
private:
	int status;

public:
    Task(QObject *parent = 0) :
    	QObject(parent), status(0) {}

	int finishStatus() { return status; }

public slots:
    void run()
    {
        using namespace ldraw::format;
        // Process ...
        try {
            qInfo() << "ldrawlibData:" << QString("0x%1").arg(intptr_t(ldrawlibData), 0, 16);
            qInfo() << "ldrawlibSize:" << ldrawlibSize;

            ldraw::archive_ro bb(ldrawlibData, ldrawlibSize);

            int i = 0, err = 0, done = 0, model = 0, index = 0;
            QElapsedTimer tm; tm.start();
            while(!bb.eof() && !done)
            {
                if (bb.get_bit_pos() != _line_info_start[index])
                    std::cerr << "Mismatch start found at " << index << ": " << bb.get_bit_pos() << "!=" << _line_info_start[index] << std::endl;

                int line_type = bb.read_bits(ID_LINE_TYPE);
                
                if (line_type != _line_info_type[index])
                    std::cerr << "Mismatch type found at " << index << ": " << line_type << "!=" << int(_line_info_type[index]) << std::endl;
                
                if (line_type == 7) {
                    const off_t offs = bb.read_bits(ID_OFFSET);
                    const off_t curr = bb.get_bit_pos();

                    bb.set_bit_pos(offs);
                    {
                        line_type = bb.read_bits(ID_LINE_TYPE);
                    }
                    bb.set_bit_pos(curr); // restore
                }
                else switch(line_type)
                {
                    case 0: {
                        int cmd = bb
                            .read_bits(ID_META_CMD);
                        if (cmd == MetaCmd_Model_End)
                            ++model;
                        if (cmd == ldraw::format::MetaCmd_Lib_End)
                            done = 1;
                    }; break;
                    case 1:
                    case 6: {
                        unsigned int col, hn;
                        float x, y, z, a=1, b=0, c=0, d=0, e=1, f=0, g=0, h=0, i=1;

                        // identity: 1 0 0 0 1 0 0 0 1
                        bb
                            .read_bits(ID_COLOR, col)
                            .read_vec(x, y, z);
                        if( line_type == 1) {
                            bb
                                .read_vec(a, b, c) // full matrix
                                .read_vec(d, e, f)
                                .read_vec(g, h, i);
                        }
                        bb
                            .read_bits(ID_NAME_HASH, hn);
                    }; break;
                    case 2: {
                        // Line
                        unsigned int col;
                        float x1, y1, z1, x2, y2, z2;
                        bb
                            .read_bits(ID_COLOR, col)
                            .read_vec(x1, y1, z1)
                            .read_vec(x2, y2, z2);
                    }; break;
                    case 3: {
                        // Triangle
                        unsigned int col;
                        float x1, y1, z1, x2, y2, z2, x3, y3, z3;
                        bb
                            .read_bits(ID_COLOR, col)
                            .read_vec(x1, y1, z1)
                            .read_vec(x2, y2, z2)
                            .read_vec(x3, y3, z3);
                    }; break;
                    case 4: {
                        // Quadrilateral
                        unsigned int col;
                        float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
                        bb
                            .read_bits(ID_COLOR, col)
                            .read_vec(x1, y1, z1)
                            .read_vec(x2, y2, z2)
                            .read_vec(x3, y3, z3)
                            .read_vec(x4, y4, z4);
                    }; break;
                    case 5: {
                        // Conditional line
                        unsigned int col;
                        float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
                        bb
                            .read_bits(ID_COLOR, col)
                            .read_vec(x1, y1, z1)
                            .read_vec(x2, y2, z2)
                            .read_vec(x3, y3, z3)
                            .read_vec(x4, y4, z4);
                    }; break;
                    default:
                        ++err;
                } // switch
                static QString fps;
                if (index%1000==0)
                    tm.restart();
                else if (index%100==0)
                    fps.sprintf(" %.2f lines/s", (index%1000)*1000/float(tm.elapsed()));
                std::cout << ("\\|/-"[index++&3]) << " " << ++i << "/" << err << " errors" << " (" << model << " models)" << fps.toStdString() << '\r'; std::cout.flush();
                if (done)
                    std::cout << std::endl << "*** Final position at " << bb.get_bit_pos() << std::endl;
            }
        } catch (const std::runtime_error &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            status = -1;
        }

        std::cout << "*** Done." << std::endl;

        emit finished();
    }

signals:
    void finished();
};

#include "tester.moc"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication::setOrganizationName("KomSoft");
    QCoreApplication::setOrganizationDomain("com.komsoft");
    QCoreApplication::setApplicationName("tester");

    // Task parented to the application so that it
    // will be deleted by the application.
    Task *task = new Task(&a);

    // This will cause the application to exit when
    // the task signals finished.    
    QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));

    // This will run the task from the application event loop.
    QTimer::singleShot(0, task, SLOT(run()));

    return a.exec();
}
