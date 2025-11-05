/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 16:36:49 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/05 16:56:26 by msuokas          ###   ########.fr       */
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
    public:
        Channel(std::string& name);
        void addClient(Client& user);
        Client getClient(std::string& clientName) const;
};