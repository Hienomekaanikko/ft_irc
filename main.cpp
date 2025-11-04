/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: msuokas <msuokas@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 16:00:10 by msuokas           #+#    #+#             */
/*   Updated: 2025/11/04 09:59:27 by msuokas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Error!" << std::endl;
    }
    else
        Server serv(std::stoi(argv[1]), argv[2]);
}
