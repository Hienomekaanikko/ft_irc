/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/04 11:06:40 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/04 11:21:28 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <netinet/in.h>

class Client {
    private:
        int _clientFd;
    public:
        Client(int _clientFd);
        void setClientFd(int fd);
        int getClientFd();

        //void sendMsg();
};
