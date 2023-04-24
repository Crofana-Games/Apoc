// Copyright Crofana Games. All Rights Reserved.

#pragma once


namespace Apocalypse
{
	enum class EAptpOpCode
	{
		/**
		 * @brief Call function. Buffer is pass directly as parameter of IFunction::Invoke().
		 */
		Call,
		/**
		 * @brief Get static/member property or enum value. Buffer is used to store the value.
		 */
		Get,
		/**
		 * @brief Set static/member property. Buffer is target value.
		 */
		Set,
	};

	struct FAptpMessage
	{
		EAptpOpCode OpCode;
		FName TypeName;
		FName MemberName;
		void* Buffer;
	};


	class FAptpProcessor
	{
	public:
		void Process(const FAptpMessage& Message) const;
	};

	const FAptpProcessor& GetAptpProcessor();
}

