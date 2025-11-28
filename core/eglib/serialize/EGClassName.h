// (c) 2018 Beem Media

#pragma once

struct eg_class_name
{
	EGClass*      Class = nullptr;
	EGClass*const BaseClass = nullptr;

	eg_class_name() = default;
	eg_class_name( EGClass* InBaseClass ): BaseClass( InBaseClass ){ }
	eg_class_name( EGClass* InBaseClass , EGClass* InAssignedClass ): BaseClass( InBaseClass ) , Class( InAssignedClass && InAssignedClass->IsA( InBaseClass ) ? InAssignedClass : nullptr ) { }

	eg_cpstr GetClassName() const { return Class ? Class->GetName() : ""; }
	void GetComboChoices( EGArray<eg_d_string>& Out ) const;
};

