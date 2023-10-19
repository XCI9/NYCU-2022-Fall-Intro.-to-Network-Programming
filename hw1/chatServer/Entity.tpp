template <typename... Strings>
void ChatServer::Entity::sendServerMsg(const Response response, Strings&&... msg) const {
    std::array<std::string_view, sizeof...(Strings)> msgList{ msg... };
    sendMsg(response, msgList);
}