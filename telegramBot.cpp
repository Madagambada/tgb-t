#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <curl/curl.h>

#include "httplib.h"
#include "strutil.h"
#include "cxxopts.hpp"

#if defined(_WIN32)
bool win = 1;
#else
bool win = 0;
#endif

std::string botToken, chatId, port, mcVersion, unaVersion, openWrtVersion, qbtVersion, testver;

httplib::Server svr;

static size_t curlCallback(void* buffer, size_t size, size_t nmemb, void* param) {
    std::string& text = *static_cast<std::string*>(param);
    size_t totalsize = size * nmemb;
    text.append(static_cast<char*>(buffer), totalsize);
    return totalsize;
}

void curlSendPost(std::string msg) {
    CURL* curl;
    CURLcode res;
    std::string data;
    struct curl_slist* list = NULL;

    std::string url = "https://api.telegram.org/bot" + botToken + "/sendMessage";
    std::string json = "{\"chat_id\": \"" + chatId + "\", \"parse_mode\": \"Markdown\", \"text\": \"" + msg + "\"}";

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        list = curl_slist_append(list, "Content-Type: application/json");
        //sudo dpkg-reconfigure ca-certificates
        if (!win) {
            curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/ssl/certs/ca-certificates.crt");
        }
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "9.9.9.9:53,149.112.112.112:53");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cout << "curlSendPost failed: " << res + ", " << curl_easy_strerror(res) << "\n";
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(list);
    }
}

std::string curlGet(std::string url) {
    CURL* curl;
    CURLcode res;
    std::string data;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        //sudo dpkg-reconfigure ca-certificates
        if (!win) {
            curl_easy_setopt(curl, CURLOPT_CAINFO, "/etc/ssl/certs/ca-certificates.crt");
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "9.9.9.9:53,149.112.112.112:53");
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cout << "curlGet failed: " << res + ", " << curl_easy_strerror(res) << "\n";
            curl_easy_cleanup(curl);
            return "curlGet failed";
        }
        curl_easy_cleanup(curl);
    }
    return data;
}

int httpServer(std::string port) {
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::string curlTest = "curlGet OK";
        if (curlGet("https://example.com") == "curlGet failed") {
            curlTest = "curlGet failed";
        }
        std::string msg = "Build with: \n\n" + std::string(curl_version()) + "\nhttplib/" + CPPHTTPLIB_VERSION + "\n\n";
        msg += "Current vars are:\nmcVersion: " + mcVersion + "\nunaVersion: " + unaVersion + "\nopenWrtVersion: " + openWrtVersion + "\nqbtVersion: " + qbtVersion + "\n\n\n'" + curlTest + "'";
        std::cout << msg;
        curlSendPost("Ping");
        res.set_content(msg, "text/plain");
    });
    svr.Post("/send", [](const httplib::Request& req, httplib::Response& res) {
        curlSendPost(req.body);
    });
    if (win) {
        svr.listen("127.0.0.1", std::stoi(port));
    }
    else {
        //Linux
        svr.listen("0.0.0.0", std::stoi(port));
     
    }
    return 0;
}

void minecraft() {
    if (mcVersion.empty()) {
        std::string curlData = curlGet("https://launchermeta.mojang.com/mc/game/version_manifest.json");
        if (curlData == "curlGet failed") {
            std::cout << "Minecraft curlGet failed\n";
            return;
        }
        curlData = strutil::split(curlData, "\"latest\": {\"release\": \"")[1];
        curlData = strutil::split(curlData, "\"")[0];
        mcVersion = curlData;
        std::cout << "Set Minecraft to: " + curlData << "\n";
        return;
    }

    std::string curlData = curlGet("https://launchermeta.mojang.com/mc/game/version_manifest.json");
    if (curlData == "curlGet failed") {
        std::cout << "Minecraft curlGet failed\n";
        return;
    }
    curlData = strutil::split(curlData, "\"latest\": {\"release\": \"")[1];
    curlData = strutil::split(curlData, "\"")[0];

    if (mcVersion != curlData) {
        std::cout << "Minecraft version detected: " + curlData << "\n";
        curlSendPost("*Minecraft " + curlData + " Relesed*");
        mcVersion = curlData;
        return;
    }
    return;
}

void una() {
    if (unaVersion.empty()) {
        std::string curlData = curlGet("https://dl.ubnt.com/unifi/debian/dists/stable/ubiquiti/binary-amd64/Packages");
        if (curlData == "curlGet failed") {
            std::cout << "UniFi Network Application curlGet failed\n";
            return;
        }
        curlData = strutil::split(curlData, "Version: ")[1];
        curlData = strutil::split(curlData, "-")[0];
        unaVersion = curlData;
        std::cout << "Set UniFi Network Application to: " + curlData << "\n";
        return;
    }

    std::string curlData = curlGet("https://dl.ubnt.com/unifi/debian/dists/stable/ubiquiti/binary-amd64/Packages");
    if (curlData == "curlGet failed") {
        std::cout << "UniFi Network Application curlGet failed\n";
        return;
    }
    curlData = strutil::split(curlData, "Version: ")[1];
    curlData = strutil::split(curlData, "-")[0];

    if (unaVersion != curlData) {
        std::cout << "New UniFi Network Application version detected: " + curlData << "\n";
        curlSendPost("*UniFi Network Application " + curlData + " Relesed*");
        unaVersion = curlData;
        return;
    }
    return;
}

void openWrt() {
    if (openWrtVersion.empty()) {
        std::string curlData = curlGet("https://downloads.openwrt.org/");
        if (curlData == "curlGet failed") {
            std::cout << "OpenWrt curlGet failed\n";
            return;
        }
        curlData = strutil::split(curlData, "Stable Release")[1];
        curlData = strutil::split(curlData, "href=\"releases/")[1];
        curlData = strutil::split(curlData, "/")[0];
        openWrtVersion = curlData;
        std::cout << "Set OpenWrt to: " + curlData << "\n";
        return;
    }

    std::string curlData = curlGet("https://github.com/openwrt/openwrt/tags");
    if (curlData == "curlGet failed") {
        std::cout << "OpenWrt curlGet failed\n";
        return;
    }
    curlData = strutil::split(curlData, "Stable Release")[1];
    curlData = strutil::split(curlData, "href=\"releases/")[1];
    curlData = strutil::split(curlData, "/")[0];

    if (openWrtVersion != curlData) {
        std::cout << "New OpenWrt version detected: " + curlData << "\n";
        curlSendPost("*OpenWrt " + curlData + " Relesed*");
        openWrtVersion = curlData;
        return;
    }
    return;
}

void qbt() {
    if (qbtVersion.empty()) {
        std::string curlData = curlGet("https://github.com/qbittorrent/qBittorrent/tags");
        if (curlData == "curlGet failed") {
            std::cout << "qbt curlGet failed\n";
            return;
        }
        curlData = strutil::split(curlData, "/qbittorrent/qBittorrent/releases/tag/release")[1];
        curlData = strutil::split(curlData, "release-")[1];
        curlData = strutil::split(curlData, "</a>")[0];
        qbtVersion = curlData;
        std::cout << "Set qBittorrent to: " + curlData << "\n";
        return;
    }

    std::string curlData = curlGet("https://github.com/qbittorrent/qBittorrent/tags");
    if (curlData == "curlGet failed") {
        std::cout << "qbt curlGet failed\n";
        return;
    }
    curlData = strutil::split(curlData, "/qbittorrent/qBittorrent/releases/tag/release")[1];
    curlData = strutil::split(curlData, "release-")[1];
    curlData = strutil::split(curlData, "</a>")[0];
    if (strutil::contains(curlData, "beta") || strutil::contains(curlData, "rc")) {
        //beta or rc release
        return;
    }
    if (qbtVersion != curlData) {
        std::cout << "New qBittorrent version detected: " + curlData << "\n";
        curlSendPost("*qBittorrent " + curlData + " Relesed*");
        qbtVersion = curlData;
    }
    return;
}

int main(int argc, char* argv[]) {
    std::cout << "Telegram Bot 1.0.0 started\n\n";
    cxxopts::Options options("Telegram Bot 1.0.0", "Sends a telegram message when an update is released.");

    options.add_options()
        ("b", "botToken", cxxopts::value<std::string>())
        ("c", "chatId", cxxopts::value<std::string>())
        ("p", "port", cxxopts::value<std::string>())
        ("v,version", "Print version")
        ("h,help", "Print usage")
        ;
    auto result = options.parse(argc, argv);

    if (result.unmatched().size() != 0) {
        std::cout << "wrong argument(s):" << std::endl;
        for (int i = 0; i < result.unmatched().size(); i++) {
            std::cout << result.unmatched()[i] << std::endl;
        }
        return 1;
    }

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count("version")) {
        std::cout << "Build with: \n\n" + std::string(curl_version()) + "\nhttplib/" + CPPHTTPLIB_VERSION + "\n\n" << std::endl;
        return 0;
    }

    if (!result.count("b") || !result.count("c") || !result.count("p")) {
        std::cout << "Error: -b, -c, -p are required arguments" << std::endl;
        return 0;
    }

    botToken = result["b"].as<std::string>();
    chatId = result["c"].as<std::string>();
    port = result["p"].as<std::string>();


    curl_global_init(CURL_GLOBAL_ALL);

    std::thread(httpServer, port).detach();
    while (!svr.is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    std::cout << "HTTP Server startet on port: " + port << "\n";
    while (svr.is_running()) {
        minecraft();
        una();
        openWrt();
        qbt();
        std::this_thread::sleep_for(std::chrono::minutes(10));
    }

    curl_global_cleanup();
    return 0;
}
