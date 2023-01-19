
// Client

///* 发送描述信息，结构：HEADER + (int)FileSize + (char*)FileName */
//HEADER filehdr;
//char filebuf[100];
//int DataSize = sizeof(len)+strlen(FileName);
//SetHeader(filehdr,0,0,0,DataSize);
//
//// 组装描述信息
//memcpy(filebuf,&filehdr,sizeof(filehdr));
//memcpy(filebuf+sizeof(filehdr),&len,sizeof(len));
//memcpy(filebuf+sizeof(filehdr)+sizeof(len),FileName,strlen(FileName));
//Add_Checksum(filebuf);
//
//sendto(s,filebuf,sizeof(filehdr)+DataSize,0,(sockaddr*)&addr,sizeof(addr));
//
//Sleep(RTO);

// Server

///* 接收描述信息，结构：HEADER + (int)FileSize + (char*)FileName */
//HEADER filehdr;
//char filebuf[100];
//recvfrom(s,filebuf,sizeof(filebuf),0,(sockaddr*)&addr,&addrlen);
//if(Test_Checksum(filebuf)){
//cout<<"描述信息接收成功..."<<endl;
//}else{
//cout<<"描述信息接收失败..."<<endl;
//return 0 ;
//}
//memcpy(&filehdr,filebuf,sizeof(filehdr));
//
//int FileSize = ((int*)(filebuf+sizeof(hdr)))[0];
//cout<<"文件大小："<<FileSize<<"Bytes"<<endl;
//
//char Filepath[] = "../recvfile/";
//char FileName[filehdr.Size-4];
//memcpy(FileName,filebuf+sizeof(filehdr)+4,filehdr.Size-4);
//FileName[filehdr.Size-4]='\0';
//
//strcat(Filepath,FileName);
//cout<<"文件路径："<<Filepath<<endl;


/* 打印文件列表 */
//    int fileNum = 1;
//    FileList fileList = Dir::entry_static("../test/");
//    for (auto &file: fileList) {
//        cout << fileNum++ << ". " << file->fileName() << endl;
//    }
//    cout << "请选择传输文件编号.." << endl;
//
//    cin >> fileNum;
//    if ((fileNum > fileList.size())) {
//        cout << "文件选择失败 " << endl;
//    }
//    auto it = fileList.begin();
//    for (; it != fileList.end(); ++it) {
//        if (--fileNum == 0) {
//            break;
//        }
//    }
//    File file = **it;

//    const char *FileName = file.fileName().c_str();
//    char Filepath[] = "../test/";
//    strcat(Filepath,FileName);
//    cout<<Filepath<<endl;