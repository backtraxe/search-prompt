#include <netinet/in.h>

#include "Trie.hpp"

class SocketInfo {
  public:
    SocketInfo(int fd, struct sockaddr_in *addr, Trie *trie,
               const int prompt_num);

    /**
     * @brief 进行通信
     *
     */
    void communicate();

  private:
    /**
     * @brief
     *
     */
    int fd;

    /**
     * @brief
     *
     */
    struct sockaddr_in *addr;

    /**
     * @brief
     *
     */
    Trie *trie;

    /**
     * @brief
     *
     */
    const int prompt_num;
};