#if !defined(NODE_KYTEA_ASYNC_H_)
#define NODE_KYTEA_ASYNC_H_
#include "node_kytea.h"
#include <string>
#include <vector>

namespace node_kytea {

class KyteaWorker : public NanAsyncWorker {
public:
    KyteaWorker(NanCallback* callback, NodeKytea* kytea);

protected:
    NodeKytea* kytea_;
};

class OpenWorker : public KyteaWorker {
public:
    OpenWorker(NanCallback* callback, NodeKytea* kytea, std::string& filename);

    /**
     * @brief Open model file
     */
    void Execute();

private:
    std::string filename_;
};

class WSWorker : public KyteaWorker {
public:
    WSWorker(NanCallback* callback, NodeKytea* kytea, std::string& text);

    /**
     * @brief Execute word segmentation.
     */
    void Execute();

    /**
     * @brief Invoke callback function
     */
    void HandleOKCallback();

private:
    std::string text_;
    kytea::KyteaSentence::Words words_;
};

class TagWorker : public KyteaWorker {
public:
    TagWorker(NanCallback* callback, NodeKytea* kytea, std::string& text, bool all = false);

    /**
     * @brief Execute particular tag.
     */
    void Execute();

    /**
     * @brief Invoke callback function
     */
    void HandleOKCallback();

private:
    std::string text_;
    kytea::KyteaSentence::Words words_;
    bool all_;
};

}

#endif // NODE_KYTEA_ASYNC_H_
