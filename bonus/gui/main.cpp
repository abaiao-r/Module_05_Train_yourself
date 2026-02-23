/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
QApplication app(argc, argv);
app.setApplicationName("Train Simulation");
app.setOrganizationName("42");

MainWindow win;
win.resize(1200, 800);
win.show();

return app.exec();
}
