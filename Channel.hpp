/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 16:36:49 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/10 15:52:41 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <iostream>
#include <vector>
#include "Client.hpp"

class Channel {
    private:
        std::string _channelName;
        std::vector<Client> _clientList;
        bool _invite_only;
        std::string _password;
    public:
        Channel(std::string& name);
        void addClient(Client& user);
        std::string& getChannelName();
        Client getClient(std::string& clientName) const;
};