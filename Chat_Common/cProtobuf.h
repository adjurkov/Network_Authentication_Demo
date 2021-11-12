#pragma once
#include "gen/authentication.pb.h"
#include <string>
#include <iostream>

// Compiled as a dynamic library
#define DLLExport __declspec ( dllexport )

class DLLExport cProtobuf
{
public:

	authentication::CreateAccountWeb* create_account_web;
	authentication::CreateAccountWebSuccess* create_account_success;
	authentication::CreateAccountWebFailure* create_account_failure;
	authentication::AuthenticateWeb* authenticate_web;
	authentication::AuthenticateWebSuccess* authenticate_success;
	authentication::AuthenticateWebFailure* authenticate_failure;

	cProtobuf();
	~cProtobuf();


	//

	//


	//tutorial::Person* person1 = address_book.add_person(); // Lucas

	//person1->set_id(1);
	//person1->set_email("l_gustafson@fanshaweonline.ca");
	//person1->set_name("Lukas Gustafson");


	//tutorial::Person::PhoneNumber* phoneNumber1 = person1->add_phone();
	//phoneNumber1->set_number("519 444 5555");
	//phoneNumber1->set_type(tutorial::Person::MOBILE);


	//std::string serializedString;
	//address_book.SerializeToString(&serializedString);

	//std::cout << serializedString << std::endl;
	//for (int idxString = 0; idxString < serializedString.length(); idxString++) {
	//	printf("%02X ", serializedString[idxString]);
	//}
	//printf("\n");

	//tutorial::AddressBook deserialized_address_book;
	//bool success = deserialized_address_book.ParseFromString(serializedString);
	//if (!success) {
	//	std::cout << "Failed to parse address book" << std::endl;
	//}
	//std::cout << "Parsing successful" << std::endl;
	//const tutorial::Person& deserialized_person = deserialized_address_book.person(i); // get person at index i
	//std::cout << "E-mail: " << deserialized_person.email() << std::endl;
	//std::cout << "Name: " << deserialized_person.name() << std::endl;





};
