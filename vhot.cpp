// VHot Compiler Core (C++20)
// バージョンシステム: 12.90
// より新しいバージョンが必要な場合はエラー

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <optional>

constexpr double VHOT_COMPILER_VERSION = 12.90;

struct CompileError {
    std::string message;
    int line;
};

class VersionManager {
public:
    static std::optional<CompileError> CheckVersion(const std::string& source) {
        std::regex versionRegex(R"(^\s*#vhot\s+([0-9]+\.[0-9]+))");
        std::smatch match;

        std::istringstream stream(source);
        std::string line;
        int lineNumber = 1;

        while (std::getline(stream, line)) {
            if (std::regex_search(line, match, versionRegex)) {
                double fileVersion = std::stod(match[1].str());
                if (fileVersion > VHOT_COMPILER_VERSION) {
                    return CompileError{
                        "このソースコードは現在のVHotでは対応していない新しい言語機能を使用しています。\n"
                        "このコードを使用するには、より新しいVHotコンパイラが必要です。\n"
                        "現在のコンパイラバージョン: " + std::to_string(VHOT_COMPILER_VERSION),
                        lineNumber
                    };
                }
                return std::nullopt;
            }
            lineNumber++;
        }

        return CompileError{ "#vhot バージョン宣言が見つかりません。ファイルの先頭に記述してください。", 1 };
    }
};

class Lexer {
public:
    static std::optional<CompileError> BasicSyntaxCheck(const std::string& source) {
        int braceCount = 0;
        int lineNumber = 1;

        for (char c : source) {
            if (c == '{') braceCount++;
            if (c == '}') braceCount--;
            if (braceCount < 0) {
                return CompileError{ "予期しない閉じ中かっこ '}' が検出されました。", lineNumber };
            }
            if (c == '\n') lineNumber++;
        }

        if (braceCount != 0) {
            return CompileError{ "中かっこの数が一致していません。開始 '{' と終了 '}' を確認してください。", lineNumber };
        }

        return std::nullopt;
    }
};

class Compiler {
public:
    static void Compile(const std::string& source) {
        if (auto versionError = VersionManager::CheckVersion(source)) {
            PrintError(*versionError);
            return;
        }

        if (auto syntaxError = Lexer::BasicSyntaxCheck(source)) {
            PrintError(*syntaxError);
            return;
        }

        std::cout << "VHot コンパイルに成功しました。 (言語バージョン: "
            << VHOT_COMPILER_VERSION
            << ")" << std::endl;
    }

private:
    static void PrintError(const CompileError& error) {
        std::cerr << "[VHot Error] 行 "
            << error.line
            << " : "
            << error.message
            << std::endl;
    }
};

std::string LoadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "ファイルを開けませんでした: " << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "使用方法: vhotc <file.vhot>\n言語バージョン: " << VHOT_COMPILER_VERSION << std::endl;
        return 1;
    }

    std::string source = LoadFile(argv[1]);
    if (source.empty()) {
        return 1;
    }

    Compiler::Compile(source);
    return 0;
}
