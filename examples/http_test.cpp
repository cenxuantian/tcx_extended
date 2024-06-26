#include <t_http_tools.hpp>
#include <t_meta.hpp>

int main(){

    tcx::Blob b;

    b <<"GET http://jsuacm.cn/ HTTP/1.1\r\n"
    "Host: jsuacm.cn\r\n"
    "Connection: keep-alive\r\n"
    "Upgrade-Insecure-Requests: 1\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.25 Safari/537.36 Core/1.70.3877.400 QQBrowser/10.8.4506.400\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: zh-CN,zh;q=0.9\r\n\r\n";

    tcx::HTTPRequest req = tcx::HTTP_read_req(b).value_or(tcx::HTTPRequest{});

    return 0;
}