/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/03/01 14:11:37 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
QApplication app(argc, argv);
app.setApplicationName("Train Simulation");
app.setOrganizationName("42");

/* Use Fusion style on all platforms so the dark stylesheet
   renders identically (macOS native style ignores some properties). */
app.setStyle(QStyleFactory::create("Fusion"));

MainWindow win;
win.resize(1200, 800);
win.show();

return app.exec();
}
