#ifndef UPDATECONFIG_H
#define UPDATECONFIG_H

#include <QString>

//namespace SWUpdate {

//class UpdateConfig
//{
//private:
//    QString d_media_path;
//    QString d_archive_regex_pattern;

//    QList<CopyFileOperation> *d_copy_operations;
//    QList<QString> *d_remove_operations;



//public:
//    UpdateConfig();
//};

//#endif // UPDATECONFIG_H


//<swupdate product="Tradinco HMI">
//    <media>
//        <media-path os="Windows"> "/media" </media-path>
//        <media-path os="Darwin"> "/media" </media-path>
//        <media-path os="Linux"> "/media" </media-path>
//        <file-match> "update*.tgz" </file-match>
//    </media>

//    <update-validate>
//        <expect type="File"> "backend" </expect>
//        <expect type="File"> "frontend" </expect>
//        <expect type="Directory"> "languages/backend" </expect>
//        <expect type="Directory"> "languages/frontend" </expect>
//        <expect type="Directory"> "languages/swupdate" </expect>
//    </update-validate>

//    <update>
//        <copy>
//            <directory root="Media"> "languages/backend" </directory>
//            <to-directory root="System"> "/etc/backend/languages" </to-directory>
//        </copy>
//        <copy>
//            <directory root="Media"> "languages/frontend" </directory>
//            <to-directory root="System"> "/etc/frontend/languages" </to-directory>
//        </copy>
//        <remove root="System" type="File"> "/appdata/appcopy/swupdate" </remove>
//    </update>
//</swupdate>

}
