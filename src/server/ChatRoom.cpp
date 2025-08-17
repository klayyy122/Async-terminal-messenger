#include "ChatRoom.hpp"
#include "ChatSession.hpp"

void ChatRoom::Connect(std::shared_ptr<ChatSession> session) {
    const std::string& username = session->getLogin();
    if (sessions_.find(username) != sessions_.end()) return;

    sessions_[username] = session;
    Broadcast(username + " joined the room\n");
}

void ChatRoom::Disconnect(std::shared_ptr<ChatSession> session) {
    auto it = sessions_.find(session->getLogin());
    if (it != sessions_.end()) {
        sessions_.erase(it);
        Broadcast(session->getLogin() + " left the room\n");
    }
}

void ChatRoom::Broadcast(const std::string& message, std::shared_ptr<ChatSession> sender) {
    for (auto it = sessions_.begin(); it != sessions_.end(); ++it) {
        if (!sender || it->second != sender) {
            it->second->deliver("[" + room_name + "] " + message);
        }
    }
}