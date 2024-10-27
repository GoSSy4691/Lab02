#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize and open the database connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QCoreApplication::applicationDirPath()+"/mydatabase.db");

    if (!db.open()) {
        qDebug() << "Failed to open the database:" << db.lastError().text();
    } else {
        qDebug() << "Database opened successfully.";

        // Create the "users" table if it does not exist
        QSqlQuery query;
        QString createTableQuery = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                surname TEXT NOT NULL,
                date TEXT NOT NULL,
                graduate TEXT NOT NULL,
                classObject TEXT NOT NULL
            )
        )";

        if (!query.exec(createTableQuery)) {
            qDebug() << "Failed to create table:" << query.lastError().text();
            return;
        } else {
            qDebug() << "Table 'users' created or already exists.";
        }
    }

    updateView();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateView()
{
    ui->listWidget->clear();
    QSqlQuery query("SELECT id, surname, date, graduate, classObject FROM users");

    // Process each row and add it to the QListWidget
    while (query.next()) {
        int id = query.value(0).toInt();
        QString surname = query.value(1).toString();
        QString date = query.value(2).toString();
        QString graduate = query.value(3).toString();
        QString classObject = query.value(4).toString();

        // Format the item text
        QString itemText = QString("ID: %1, Surname: %2, Date: %3, Graduate: %4, Class: %5")
                               .arg(id)
                               .arg(surname)
                               .arg(date)
                               .arg(graduate)
                               .arg(classObject);

        // Add the item to the QListWidget
        ui->listWidget->addItem(itemText);
    }
}

void MainWindow::on_add_clicked()
{
    // Retrieve text from input fields
    QString surname = ui->surname->text();
    QString date = ui->date->text();
    QString graduate = ui->graduate->text();
    QString classObject = ui->classObject->text();

    // Prepare an SQL query to insert data into the 'users' table
    QSqlQuery query;
    query.prepare("INSERT INTO users (surname, date, graduate, classObject) "
                  "VALUES (:surname, :date, :graduate, :classObject)");

    // Bind values to prevent SQL injection
    query.bindValue(":surname", surname);
    query.bindValue(":date", date);
    query.bindValue(":graduate", graduate);
    query.bindValue(":classObject", classObject);

    // Execute the query and check if it was successful
    if (!query.exec()) {
        qDebug() << "Error adding data to database:" << query.lastError().text();
        return;
    } else {
        qDebug() << "Data added to database successfully.";
    }

    // Retrieve the ID of the last inserted row
    int lastId = query.lastInsertId().toInt();

    // Format the new item text
    QString itemText = QString("ID: %1, Surname: %2, Date: %3, Graduate: %4, Class: %5")
                           .arg(lastId)
                           .arg(surname)
                           .arg(date)
                           .arg(graduate)
                           .arg(classObject);

    // Add the new item to the QListWidget
    ui->listWidget->addItem(itemText);
}


void MainWindow::on_close_clicked()
{
    qApp->exit();
}

void MainWindow::on_remove_clicked()
{
    QListWidgetItem *currentItem = ui->listWidget->currentItem();

    if (currentItem) {
        // Extract the ID from the item's text (assuming ID is the first part of the text, e.g., "ID: 1, ...")
        QString itemText = currentItem->text();
        int id = itemText.split(" ")[1].replace(",", "").toInt();  // Extracts ID assuming format is "ID: <id>"

        // Prepare a query to delete the row with the extracted ID
        QSqlQuery query;
        query.prepare("DELETE FROM users WHERE id = :id");
        query.bindValue(":id", id);

        // Execute the delete query
        if (query.exec()) {
            qDebug() << "Item removed from database successfully.";

            // Remove the item from the QListWidget
            delete currentItem;
        } else {
            qDebug() << "Error removing item from database:" << query.lastError().text();
        }
    } else {
        qDebug() << "No item selected to remove.";
    }
}

void MainWindow::on_search_clicked()
{
    // Clear the list widget before showing filtered results
    ui->listWidget->clear();

    // Retrieve text from input fields
    QString surname = ui->surname->text();
    QString date = ui->date->text();
    QString graduate = ui->graduate->text();
    QString classObject = ui->classObject->text();

    // Prepare the SQL query with placeholders for each filter
    QString queryStr = "SELECT id, surname, date, graduate, classObject FROM users "
                       "WHERE surname LIKE :surname AND date LIKE :date "
                       "AND graduate LIKE :graduate AND classObject LIKE :classObject";

    QSqlQuery query;
    query.prepare(queryStr);

    // Bind each value, using '%' for empty fields to match all values
    query.bindValue(":surname", surname.isEmpty() ? "%" : "%" + surname + "%");
    query.bindValue(":date", date.isEmpty() ? "%" : "%" + date + "%");
    query.bindValue(":graduate", graduate.isEmpty() ? "%" : "%" + graduate + "%");
    query.bindValue(":classObject", classObject.isEmpty() ? "%" : "%" + classObject + "%");

    // Execute the query and populate the list widget with results
    if (query.exec()) {
        while (query.next()) {
            int id = query.value("id").toInt();
            QString surname = query.value("surname").toString();
            QString date = query.value("date").toString();
            QString graduate = query.value("graduate").toString();
            QString classObject = query.value("classObject").toString();

            QString itemText = QString("ID: %1, Surname: %2, Date: %3, Graduate: %4, Class: %5")
                                   .arg(id)
                                   .arg(surname)
                                   .arg(date)
                                   .arg(graduate)
                                   .arg(classObject);

            ui->listWidget->addItem(itemText);
        }
    } else {
        qDebug() << "Error executing search query:" << query.lastError().text();
    }
}


void MainWindow::on_viewAll_clicked()
{
    updateView();
}
