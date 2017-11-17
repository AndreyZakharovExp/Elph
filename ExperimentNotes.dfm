object ExpNotes: TExpNotes
  Left = 350
  Top = 200
  Anchors = [akLeft, akTop, akRight]
  BorderStyle = bsToolWindow
  Caption = #1048#1085#1092#1086#1088#1084#1072#1094#1080#1103' '#1086#1073' '#1101#1082#1089#1087#1077#1088#1080#1084#1077#1085#1090#1077
  ClientHeight = 175
  ClientWidth = 289
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesigned
  ExplicitWidth = 320
  DesignSize = (
    289
    175)
  PixelsPerInch = 96
  TextHeight = 13
  object mainLinesLbl: TLabel
    Left = 42
    Top = 4
    Width = 209
    Height = 16
    Caption = #1054#1073#1097#1080#1077' '#1089#1074#1077#1076#1077#1085#1080#1103' '#1086#1073' '#1101#1082#1089#1087#1077#1088#1080#1084#1077#1085#1090#1077
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object addMarkLbl: TLabel
    Left = 35
    Top = 133
    Width = 214
    Height = 16
    Anchors = [akLeft]
    Caption = #1048#1085#1092#1086#1088#1084#1072#1094#1080#1103' '#1087#1086' '#1093#1086#1076#1091' '#1101#1082#1089#1087#1077#1088#1080#1084#1077#1085#1090#1072
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
  end
  object usersNotes: TMemo
    Left = 0
    Top = 22
    Width = 288
    Height = 103
    Anchors = [akLeft, akTop, akRight, akBottom]
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'Arial'
    Font.Style = []
    MaxLength = 256
    ParentFont = False
    TabOrder = 0
  end
  object PIDates: TMemo
    Left = 127
    Top = 67
    Width = 74
    Height = 62
    MaxLength = 64
    ReadOnly = True
    TabOrder = 1
    Visible = False
  end
  object addUMark: TEdit
    Left = 0
    Top = 150
    Width = 288
    Height = 23
    Anchors = [akLeft, akRight]
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'Arial'
    Font.Style = []
    ParentFont = False
    TabOrder = 2
    OnKeyPress = addUMarkKeyPress
  end
end
