/*
  ==============================================================================

    PasswordPropertyComponent.h
    Created: 12 Jan 2017 9:35:48am
    Author:  LoopFine

  ==============================================================================
*/

#ifndef PASSWORDPROPERTYCOMPONENT_H_INCLUDED
#define PASSWORDPROPERTYCOMPONENT_H_INCLUDED

//==============================================================================
/**
A PropertyComponent for display and input password. 

    - It'll display the original characters when it's been editing.
    - The max-length of the password must not more than 120.

@see PropertyComponent
*/
class PasswordPropertyComponent : public PropertyComponent
{
public:
    /** Creates a password property component.

    @param valueToControl The Value that is controlled by this object
    @param propertyName   The name of the property which will display on the left
    */
    PasswordPropertyComponent(const Value& valueToControl,
                              const String& propertyName);

    /** Destructor. */
    ~PasswordPropertyComponent();

    //==============================================================================
    /** Called when the user edits the text. */
    virtual void setText(const String& newText);

    /** Returns the text that should be shown in the text editor. */
    virtual String getText() const;

    /** Returns the text that should be shown in the text editor as a Value object. */
    Value& getValue() const;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

    These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
    methods.

    @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId = 0x100e401,    /**< The colour to fill the background of the text area. */
        textColourId = 0x100e402,          /**< The colour to use for the editable text. */
        outlineColourId = 0x100e403,       /**< The colour to use to draw an outline around the text area. */
    };

    void colourChanged() override;

    //==============================================================================
    class Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() { }

        /** Called when text has finished being entered (i.e. not per keypress) has changed. */
        virtual void textPropertyComponentChanged(PasswordPropertyComponent*) = 0;
    };

    /** Registers a listener to receive events when this button's state changes.
    If the listener is already registered, this will not register it again.
    @see removeListener
    */
    void addListener(Listener* newListener);

    /** Removes a previously-registered button listener
    @see addListener
    */
    void removeListener(Listener* listener);

    //==============================================================================
    /** @internal */
    void refresh() override;
    /** @internal */
    virtual void textWasEdited();

private:
    class LabelComp;
    friend class LabelComp;

    ScopedPointer<LabelComp> textEditor;
    ListenerList<Listener> listenerList;

    void callListeners();
    void createEditor();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PasswordPropertyComponent)
};

#endif  // PASSWORDPROPERTYCOMPONENT_H_INCLUDED

