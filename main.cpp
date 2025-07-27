#include <iostream>
#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

const std::string CONNECTION_STRING =
"postgresql://admin:gcW10R2_HiftI07pc-D0TzghDVs1mp@asia-south1.509ecc4c-201c-4248-a364-c059af51f5c4.gcp.yugabyte.cloud:5433/yugabyte?ssl=true&sslmode=verify-full&sslrootcert=root.crt";
// "postgresql://postgres:0909@localhost:5432/yugabyte";

Session* connect();
void createDatabase(Session* session);
void selectAccounts(Session* session);
void transferMoneyBetweenAccounts(Session* session, int amount);

int main(int argc, char* argv[]) {
    std::cout << "Welcome!" << std::endl;
    Poco::Data::PostgreSQL::Connector::registerConnector();
    Session* session = nullptr;
    try {
        std::cout << "Using connection string: " << CONNECTION_STRING << std::endl;
        session = connect();

        createDatabase(session);
        selectAccounts(session);
        transferMoneyBetweenAccounts(session, 800);
        selectAccounts(session);

    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;

        if (session != nullptr) {
            delete session;
        }
        Poco::Data::PostgreSQL::Connector::unregisterConnector();
        return 1;
    }

    if (session != nullptr) {
        delete session;
    }
    Poco::Data::PostgreSQL::Connector::unregisterConnector();

    return 0;
}

Session* connect() {
    std::cout << ">>>> Connecting to YugabyteDB!" << std::endl;
    Session* session = new Session("PostgreSQL", CONNECTION_STRING);
    std::cout << ">>>> Successfully connected to YugabyteDB!" << std::endl;
    return session;
}

void createDatabase(Session* session) {
    std::cout << ">>>> Creating tables..." << std::endl;
    *session << "DROP TABLE IF EXISTS DemoAccount", now;
    *session << "CREATE TABLE DemoAccount ( \
                id int PRIMARY KEY, \
                name varchar, \
                age int, \
                country varchar, \
                balance int)", now;
    *session << "INSERT INTO DemoAccount VALUES (1, 'Jessica', 28, 'USA', 10000)", now;
    *session << "INSERT INTO DemoAccount VALUES (2, 'John', 28, 'Canada', 9000)", now;

    std::cout << ">>>> Successfully created table DemoAccount." << std::endl;
}

void selectAccounts(Session* session) {
    std::cout << ">>>> Selecting accounts:" << std::endl;

    Statement select(*session);
    select << "SELECT name, age, country, balance FROM DemoAccount";
    select.execute();

    Poco::Data::RecordSet rs(select);

    for (size_t i = 0; i < rs.rowCount(); ++i) {
        std::string name = rs["name"].convert<std::string>();
        int age = rs["age"].convert<int>();
        std::string country = rs["country"].convert<std::string>();
        int balance = rs["balance"].convert<int>();

        std::cout << "name=" << name << ", "
            << "age=" << age << ", "
            << "country=" << country << ", "
            << "balance=" << balance << std::endl;
        rs.moveNext();
    }
}

void transferMoneyBetweenAccounts(Session* session, int amount) {
    std::cout << ">>>> Transferring " << amount << " between accounts..." << std::endl;
    try {
        *session << "UPDATE DemoAccount SET balance = balance -" + std::to_string(amount) +
            " WHERE name = 'Jessica'", now;
        *session << "UPDATE DemoAccount SET balance = balance +" + std::to_string(amount) +
            " WHERE name = 'John'", now;

        std::cout << ">>>> Transferred " << amount << " between accounts." << std::endl;
    }
    catch (const Poco::Exception& e) {
        std::cerr << "Database error: " << e.displayText() << std::endl;
        throw;
    }
}