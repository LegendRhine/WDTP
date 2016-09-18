/*
  ==============================================================================

    PasswordPropertyComponent.h
    Created: 18 Sep 2016 9:35:48am
    Author:  SwingCoder

  ==============================================================================
*/

#ifndef PASSWORDPROPERTYCOMPONENT_H_INCLUDED
#define PASSWORDPROPERTYCOMPONENT_H_INCLUDED

//==============================================================================
/**
用于设置密码的单行TextPropertyComponent.

@see PropertyComponent
*/
class   PasswordPropertyComponent : public PropertyComponent
{
protected:
    //==============================================================================
    /** 创建一个文本属性组件.

    @param propertyName  属性名

    @see TextEditor
    */
    PasswordPropertyComponent (const String& propertyName);

public:
    /** 创建一个文本属性组件.

    @param valueToControl 所关联控制的值
    @param propertyName   属性名

    @see TextEditor
    */
    PasswordPropertyComponent (const Value& valueToControl,
                           const String& propertyName);

    /** 析构函数. */
    ~PasswordPropertyComponent ();

    //==============================================================================
    /** 设置文本。用户编辑文本时，自动调用此方法。
    */
    virtual void setText (const String& newText);

    /** 返回当前显示在文本编辑器中的文本.*/
    virtual String getText () const;

    /** 将文本编辑器中的内容返回为 Value 对象. */
    Value& getValue () const;

    //==============================================================================
    /** 此组件的各元素的颜色ID.

    此枚举常量用于 Component::setColour(), 或 LookAndFeel::setColour().

    @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId = 0x100e401,    /**< 背景色ID. */
        textColourId = 0x100e402,          /**< 文本色ID. */
        outlineColourId = 0x100e403,       /**< 边线颜色ID. */
    };

    void colourChanged () override;

    //==============================================================================
    /** @internal */
    void refresh () override;
    
    virtual void textWasEdited();
    virtual void focusOfChildComponentChanged (FocusChangeType cause) override;

private:
    void createEditor ();
    ScopedPointer<TextEditor> textEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PasswordPropertyComponent)
};


#endif  // HEADERGUA

