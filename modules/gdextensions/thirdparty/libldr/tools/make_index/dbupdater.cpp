
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <random>

#include <QDebug>
#include <QSize>
#include <QDir>
#include <QDirIterator>
#include <QTimer>
#include <QFile>
#include <QSet>
#include <QStringList>
#include <QStandardPaths>
#include <QPixmap>
#include <QApplication>
#include <QMessageBox>


#include <libldr/color.h>
#include <libldr/metrics.h>
#include <libldr/model.h>
#include <libldr/part_library.h>
#include <libldr/reader.h>
#include <libldr/lutils.h>

#include "config.h"

#define DB_REVISION_NUMBER 3

namespace mkindex
{


class DBUpdater : public QObject
{
 public:
  DBUpdater(const std::string &path, QObject *parent = 0L);
  ~DBUpdater();

  void deleteAll();
  int start();

 private:
  QString saveLocation(const QString &path);

  static QString escape(const QString &string);
  static QString unescape(const QString &string);

  void determineSize(const QString &str, float &xs, float &ys, float &zs);
  float floatify(const QString &str);

  Config *config_;
  ldraw::part_library *library_;
  ldraw::reader *reader_;

  bool status_, saveloc_;
};


DBUpdater::DBUpdater(const std::string &path, QObject *parent)
    : QObject(parent)
{
  config_ = 0L;
  reader_ = 0L;
  library_ = 0L;
  status_ = saveloc_ = false;

  try {
    library_ = new ldraw::part_library(path);
  } catch (const ldraw::exception &e) {
    std::cerr << e.what() << std::endl;
    return;
  }

  status_ = true;
  
  library_->set_unlink_policy(ldraw::part_library::parts);
  ldraw::color::init();
  reader_ = new ldraw::reader(library_->ldrawpath(ldraw::part_library::ldraw_parts_path));
  reader_->enable_global_cache();

  config_ = new Config;
}

DBUpdater::~DBUpdater()
{
  if (config_) delete config_;
  if (reader_) delete reader_;
  if (library_) delete library_;
}

void DBUpdater::deleteAll()
{
  config_->setPartCount(-1);
  config_->writeConfig();
}

int DBUpdater::start()
{
  if (!status_)
    return 1;
  
  QHash<QString, int> categories;
  
  QDir dir(library_->ldrawpath(ldraw::part_library::ldraw_parts_path).c_str());
  
  const std::map<std::string, std::string> &partlist = library_->part_list();
  int totalSize = partlist.size();

  QString dbfile = saveLocation("") + "parts.db";

  std::cout << "[dbupdater] Save location: " << dbfile.toLocal8Bit().data() << std::endl;
  std::cout << "[dbupdater] Parts library: " << totalSize << " elements" << std::endl;

  deleteAll();

  // quick tests:
  class rand_t {
    public:
      std::random_device rd;
      std::mt19937 mt;
      std::uniform_real_distribution<double>  dist;
      rand_t() : rd{}, mt{rd()}, dist{ldraw::format::Nmin, ldraw::format::Nmax} {}
      float rand() { return dist(mt); }
  } rnd;
  for(auto &v : std::vector<float>{
      ldraw::format::Nmin-1,
      ldraw::format::Nmin,
      ldraw::format::Nmax,
      123.55,
      47.059,
      rnd.rand(),
      rnd.rand(),
      rnd.rand(),
      rnd.rand(),
  }) {
    std::cout << v << " = " << ldraw::archive::float_from_bits(ldraw::archive::float_to_bits(v)) << std::endl;
  }
  // end.

  int i = 0;
  ldraw::archive bb;
  ldraw::reader::archiver(&bb);

  for (std::map<std::string, std::string>::const_iterator it = partlist.begin(); it != partlist.end(); ++it, ++i) {

    QString qFilename = QString((*it).second.c_str());
    int fsize = (int)QFileInfo(dir, qFilename).size();
    bool insert = true;
    int idx = 0;

    // load the model
    ldraw::item_refcount *n;
    try {
      n = reader_->load_with_cache((*it).second);
    } catch (const ldraw::exception &e) {
      std::cerr << e.what() << std::endl;
      continue;
    }

    ldraw::model_multipart *m = n->model();

    // If current part is a link to other one, skip it.
    if (ldraw::utils::translate_string(m->main_model()->desc()).find("moved to") != std::string::npos ||
        m->main_model()->desc()[0] == '~') {
      // continue;
    }
    
#ifdef VERBOSE
    std::cout << i << " " << totalSize - 1 << " " << m->main_model()->name() << " (" << m->main_model()->desc() << ")" << std::endl;
#else
    static int index = 0;
    std::cout << ("\\|/-"[index++&3]) << " " << i << "/" << totalSize << '\r'; std::cout.flush();
#endif

    library_->link(m);
    
    // Render and save
    ldraw::utils::validate_bowtie_quads(m->main_model());

    // Metrics
    if (!m->main_model()->custom_data<ldraw::metrics>())
      m->main_model()->update_custom_data<ldraw::metrics>();
    const ldraw::metrics *metrics = m->main_model()->custom_data<ldraw::metrics>();
    const ldraw::vector &min = metrics->min_();
    const ldraw::vector &max = metrics->max_();
    
    QString qPartno = escape(qFilename.section('.', 0, 0));
    QString qDesc = escape(m->main_model()->desc().c_str());
    
    // Check whether this part is official or unofficial
    int unofficial = 0;
    std::list<std::string> ldraworgheader = m->main_model()->header("LDRAW_ORG");
    if (ldraworgheader.size() > 0 && ldraw::utils::translate_string(*ldraworgheader.begin()).find("unofficial") != std::string::npos)
      unofficial = 1;
    
    // Regular expression
    float xs, ys, zs;
    determineSize(qDesc, xs, ys, zs);

    // QString("INSERT INTO parts(partid, desc, filename, xsize, ysize, ") +
    // QString("zsize, minx, maxx, miny, maxy, minz, maxz, size, magic, unofficial) ") +
    // QString("VALUES('%1', '%2', '%3', %4, %5, %6, ").arg(qPartno, qDesc, escape(qFilename)).arg(xs).arg(ys).arg(zs) +
    // QString("%1, %2, %3, %4, %5, %6, ").arg(min.x()).arg(max.x()).arg(min.y()).arg(max.y()).arg(min.z()).arg(max.z()) +
    // QString("%1, %2, %3)").arg(fsize).arg(config_->magic()).arg(unofficial);

    // Categories
    QSet<QString> setCats;
    std::list<std::string> catheader = m->main_model()->header("CATEGORY");
    
    QString qCategory = qDesc.section(' ', 0, 0);
    if (qCategory[0] == '_' || qCategory[0] == '~') // Colored parts
      qCategory = qCategory.right(qCategory.length()-1);
    setCats.insert(qCategory);
    for (std::list<std::string>::iterator it = catheader.begin(); it != catheader.end(); ++it)
      setCats.insert(QString((*it).c_str()).trimmed());
  }

  // end-marker
  bb
    .line_info(0)
    .write_bits(0, ldraw::format::ID_LINE_TYPE).profile("ID_LINE_TYPE", 0)
    .write_bits(ldraw::format::MetaCmd_Lib_End, ldraw::format::ID_META_CMD);

  std::cout << std::endl << (totalSize - 1) << " " << (totalSize - 1) << " Finished" << std::endl;
  
  std::ofstream fdb(dbfile.toStdString(), std::ios::out | std::ofstream::binary);
  const auto &bytes = bb.get_bytes();
  std::copy(bytes.begin(), bytes.end(), std::ostreambuf_iterator<char>(fdb));

  ldraw::color::save_index();

  std::cout << "Archived " << bytes.size() << " bytes, " << "bit position " << bb.get_bit_pos() << std::endl;

  config_->setDatabaseRevision(DB_REVISION_NUMBER);
  config_->setPartCount(totalSize);
  config_->setMagic(config_->magic() + 1);
  config_->writeConfig();
  
  return 0;
}

QString DBUpdater::saveLocation(const QString &directory)
{
  QString result = (saveloc_?(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/"):"./") + directory;

  QDir().mkpath(result);
  
  return result;
}

QString DBUpdater::escape(const QString &string)
{
  QString n = string;
  n.replace('\'', "''");
  
  return n;
}

QString DBUpdater::unescape(const QString &string)
{
  QString r = string;
  r.replace("''", "'");
  return r;
}

void DBUpdater::determineSize(const QString &str, float &xs, float &ys, float &zs)
{
  static QRegExp triplet("([./\\d]+) *x *([./\\d]+) *x *([./\\d]+)");
  static QRegExp pair("([./\\d]+) *x *([./\\d]+)");
  static QRegExp single("([./\\d]+)");
  
  if (triplet.indexIn(str) > -1) {
    xs = floatify(triplet.cap(1));
    ys = floatify(triplet.cap(2));
    zs = floatify(triplet.cap(3));
  } else if (pair.indexIn(str) > -1) {
    xs = floatify(pair.cap(1));
    ys = floatify(pair.cap(2));
    zs = 0.0f;
  } else {
    int pos = 0;
    xs = 0.0f;
    while ((pos = single.indexIn(str, pos)) != 1) {
      xs = floatify(single.cap(1));
      if (xs < 32.0f) // empirical
        break;
      pos += single.matchedLength();
    }
    ys = 0.0f;
    zs = 0.0f;
  }
}

float DBUpdater::floatify(const QString &str)
{
  if (str.contains('/'))
    return str.section('/', 0, 0).toFloat() / str.section('/', 1, 1).toFloat();
  else
    return str.toFloat();
}

} // mkindex



class Task : public QObject
{
    Q_OBJECT
private:
    const QString path;
    int status;
    
public:
    Task(const QString &path, QObject *parent = 0) :
    QObject(parent), path(path), status(0) {}
    
    int finishStatus() { return status; }
    
    public slots:
    void run()
    {
        // Process ...
        
        try {
            std::string p = path.toLocal8Bit().data();
            mkindex::DBUpdater updater(p);
            status = updater.start();
        } catch (const std::runtime_error &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            status = -1;
        }
        
        emit finished();
    }
    
signals:
    void finished();
};

#include "dbupdater.moc"

void usage(const char *progname)
{
    std::cerr << "Usage: " << progname << " path-to-ldraw" << std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QCoreApplication::setOrganizationName("KomSoft");
    QCoreApplication::setOrganizationDomain("com.komsoft");
    QCoreApplication::setApplicationName("mkindex");
    
    QStringList args = a.arguments();
    QString path;
    
    for (int i = 1; i < args.count(); ++i) {
        const QString &arg = args[i];
        
        if (arg.startsWith("-")) {
            std::cerr << "Unrecognized option: " << arg.toLocal8Bit().data() << std::endl;
            usage(argv[0]);
            return 1;
        } else {
            path = arg;
        }
    }
    
    if (path.isEmpty()) {
        usage(argv[0]);
        return 1;
    }
    
    // Task parented to the application so that it
    // will be deleted by the application.
    Task *task = new Task(path, &a);
    
    // This will cause the application to exit when
    // the task signals finished.
    QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));
    
    // This will run the task from the application event loop.
    QTimer::singleShot(0, task, SLOT(run()));
    
    return a.exec();
}
