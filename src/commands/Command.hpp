/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsadouk <tsadouk@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 00:09:12 by tsadouk           #+#    #+#             */
/*   Updated: 2025/02/04 00:13:22 by tsadouk          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
# define COMMAND_HPP

# include <string>
# include <vector>

struct Command {
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
};

class CommandParser {
	public:
		static Command parseCommand(const std::string& message);
};


#endif