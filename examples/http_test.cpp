#include <t_math.hpp>
#include <t_http.hpp>
// #include <t_meta.hpp>
#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <t_file.hpp>
#include <t_url.hpp>

int main(){    
    tcx::HTTPClient client(tcx::IPAddr::url("www.google.com"));
    tcx::HTTPRequest req;
    req.type = tcx::HTTPReqType::T_GET;
    req.headers.emplace("connection","conn");
    auto res = client.send(req);
    if(res.has_value()){
        tcx::Blob& body = res.value().body;
        std::cout << body.c_str() << '\n';
        std::string path = tcx::Path::cwd().cd("..").cd("./test.html").str();
        tcx::overwrite(path,(char*)body.data(),body.size());
    }
    else{
        std::cout << GetLastError() << '\n';
        return 0;
    }

    // tcx::Blob b;

    // b <<"GET http://jsuacm.cn/ HTTP/1.1\r\n"
    // "Host: jsuacm.cn\r\n"
    // "Connection: keep-alive\r\n"
    // "Upgrade-Insecure-Requests: 1\r\n"
    // "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.25 Safari/537.36 Core/1.70.3877.400 QQBrowser/10.8.4506.400\r\n"
    // "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"
    // "Accept-Encoding: gzip, deflate\r\n"
    // "Accept-Language: zh-CN,zh;q=0.9\r\n\r\n";

    // tcx::HTTPRequest req = tcx::HTTP_read_req(b).value_or(tcx::HTTPRequest{});
    // auto str = tcx::HTTP_to_blob(req);
    // std::cout << str.data();

    // b.clear();

    // b << "HTTP/1.1 200 OK\r\n"
    //     "Server: nginx/1.18.0 (Ubuntu)\r\n"
    //     "Date: Wed, 20 Oct 2021 06:46:15 GMT\r\n"
    //     "Content-Type: text/html; charset=UTF-8\r\n"
    //     "Connection: keep-alive\r\n"
    //     "Expires: Thu, 19 Nov 1981 08:52:00 GMT\r\n"
    //     "Cache-Control: no-store, no-cache, must-revalidate\r\n"
    //     "Pragma: no-cache\r\n"
    //     "Content-Length: 737265\r\n\r\n";

    // tcx::HTTPResponse res = tcx::HTTP_read_res(b).value_or(tcx::HTTPResponse{});
    // str.clear();
    // str << tcx::HTTP_to_blob(res);
    //  std::cout << str.data();

    return 0;
}

