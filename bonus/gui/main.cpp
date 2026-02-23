/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by ctw03933          #+#    #+#             */
/*   Updated: 2026/02/22 18:34:31 by ctw03933         ###   ########.fr       */
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
