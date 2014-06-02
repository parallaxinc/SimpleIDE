#ifndef ZIPPER_H
#define ZIPPER_H

#include <QtWidgets>
#include "StatusDialog.h"

#if 0
extern "C" {
    extern int minizip(const char *zipFileName, const char *fileListCSV);
    extern int miniunzip(const char *zipFile, const char *extractToFile);
}
#endif

class Zipper : public QObject
{
    Q_OBJECT
public:
    explicit Zipper(QObject *parent = 0);
    // special spinzip
    bool makeSpinZip(QString fileName, QStringList fileTree, QString libPath, StatusDialog *stat);
    // any zip
    bool zipit(QString fileName, QString folder);
    bool zipFileList(QString source, QStringList list, QString dstZipFile);

    bool unzipAll(QString fileName, QString folder, QString special = "");
    QString unzipFirstFile(QString zipName, QString *fileName);
    QString unzipFile(QString zipName, QString fileName);
    bool unzipFileExists(QString zipName, QString fileName);
    int  unzipFileCount(QString zipName);

    QStringList directoryTreeList(QString folder);

private:
    QString getZipDestination(QString fileName);
    void    zipSpinProjectTree(QString fileName, QStringList fileTree);
    bool    createFolderZip(QString source, QString dstZipFile);
    QString filePathName(QString fileName);
    QString shortFileName(QString fileName);

    QString spinLibPath;
    StatusDialog *statusDialog;

signals:

public slots:

};

#endif // ZIPPER_H
