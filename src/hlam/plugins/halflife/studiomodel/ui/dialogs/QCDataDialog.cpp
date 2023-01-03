#include <algorithm>
#include <cassert>

#include <QApplication>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QLabel>
#include <QPlainTextEdit>
#include <QToolBox>
#include <QWidget>

#include "ui_QCDataDialog.h"

#include "formats/activity.hpp"
#include "formats/studiomodel/StudioModelUtils.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/dialogs/QCDataDialog.hpp"

using namespace studiomdl;

namespace studiomodel
{
QCDataDialog::QCDataDialog(StudioModelAssetProvider* provider, QWidget* parent)
	: QDialog(parent)
	, _ui(std::make_unique<Ui_QCDataDialog>())
	, _provider(provider)
	, _asset(_provider->GetCurrentAsset())
{
	_ui->setupUi(this);

	//Set the dialog size to a fraction of the current screen
	const QRect screenSize = QApplication::desktop()->screenGeometry(this);

	const int height = static_cast<int>(std::ceil(screenSize.height() * 0.9));

	this->setMinimumHeight(height);

	this->resize(static_cast<int>(std::ceil(screenSize.width() * 0.9)), height);

	const auto model = _asset->GetEditableStudioModel();

	auto toolbox = new QToolBox(this);

	_ui->Layout->addWidget(toolbox);

	const auto initializePage = [&]<typename TContainer>(const QString& name, const QString& description,
		const TContainer& container, bool createTextEdit) -> QPlainTextEdit*
	{
		auto widget = new QWidget(toolbox);

		toolbox->addItem(widget, QString{"%1 (%2)"}.arg(name).arg(container.size()));

		auto layout = new QVBoxLayout(widget);

		widget->setLayout(layout);

		layout->setContentsMargins(0, 0, 0, 0);

		if (!description.isEmpty())
		{
			auto descriptionLabel = new QLabel(description, widget);
			descriptionLabel->setTextFormat(Qt::RichText);
			layout->addWidget(descriptionLabel);
		}

		if (createTextEdit)
		{
			auto textEdit = new QPlainTextEdit(widget);

			textEdit->setReadOnly(true);
			textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
			textEdit->setFont(QFont{"Consolas", 8});

			layout->addWidget(textEdit);

			return textEdit;
		}
		else
		{
			layout->addStretch(0);
		}

		return nullptr;
	};

	const auto initializePageWithTextEdit = [&]<typename TContainer, typename TCallback>(
		const QString& name, const QString& description, const TContainer& container, TCallback&& callback)
	{
		auto textEdit = initializePage(name, description, container, true);

		callback(textEdit);

		textEdit->moveCursor(QTextCursor::Start);
	};

	const auto initializePageWithList = [&]<typename TContainer, typename TCallback>(
		const QString& name, const QString& description, const TContainer& container, TCallback&& callback)
	{
		initializePageWithTextEdit(name, description, container,
			[&](QPlainTextEdit* textEdit)
			{
				for (int index = 0; const auto & element : container)
				{
					textEdit->appendPlainText(callback(index, element));
					++index;
				}
			});
	};

	initializePageWithTextEdit(
		"Bones", "Bones are defined in body smd files referenced by <b>$body</b> and <b>$bodygroup</b> commands",
		model->Bones,
		[&](QPlainTextEdit* textEdit)
		{
			textEdit->appendPlainText("nodes");

			for (int index = 0; const auto& bone : model->Bones)
			{
				textEdit->appendPlainText(QString{"%1 \"%2\" %3"}
					.arg(index, 3)
					.arg(QString::fromStdString(bone->Name))
					.arg(bone->Parent ? bone->Parent->ArrayIndex : -1, 3));
				++index;
			}
			
			textEdit->appendPlainText("end");
		});

	initializePageWithList("Bone Controllers", {}, model->BoneControllers,
		[&](int index, const auto& boneController)
		{
			QString result{"$controller %1"};

			if (boneController->Index == STUDIO_MOUTH_CONTROLLER)
			{
				result = result.arg("mouth");
			}
			else
			{
				result = result.arg(boneController->Index);
			}

			if (boneController->ArrayIndex == -1)
			{
				return result;
			}

			return QString{"%1 \"%2\" %3 %4 %5"}
				.arg(result)
				.arg(QString::fromStdString(model->Bones[boneController->ArrayIndex]->Name))
				.arg(ControlToString(boneController->Type))
				.arg(boneController->Start)
				.arg(boneController->End);
		});

	initializePageWithList("Body Parts", {}, model->Bodyparts,
		[&](int index, const auto& bodypart)
		{
			if (bodypart->Models.size() == 1)
			{
				// Single body
				return QString{"$body studio \"%1\""}.arg(QString::fromStdString(bodypart->Models[0].Name));
			}
			else
			{
				// Body group
				QString result = QString{"$bodygroup %1\n{"}.arg(QString::fromStdString(bodypart->Name));

				for (const auto& subModel : bodypart->Models)
				{
					if (subModel.Name == "blank" && subModel.Meshes.empty())
					{
						result += "\nblank";
					}
					else
					{
						result += QString{"\nstudio \"%1\""}.arg(QString::fromStdString(subModel.Name));
					}
				}

				result += "\n}";

				return result;
			}
		});

	initializePageWithList("Hitboxes", {}, model->Hitboxes,
		[&](int index, const auto& hitbox)
		{
			return QString{"$hbox %1 \"%2\" %3 %4 %5 %6 %7 %8"}
				.arg(hitbox->Group)
				.arg(QString::fromStdString(model->Bones[hitbox->Bone->ArrayIndex]->Name))
				.arg(hitbox->Min.x, 0, 'f', 2)
				.arg(hitbox->Min.y, 0, 'f', 2)
				.arg(hitbox->Min.z, 0, 'f', 2)
				.arg(hitbox->Max.x, 0, 'f', 2)
				.arg(hitbox->Max.y, 0, 'f', 2)
				.arg(hitbox->Max.z, 0, 'f', 2);
		});

	initializePageWithList("Attachments", {}, model->Attachments,
		[&](int index, const auto& attachment)
		{
			return QString{"$attachment %1 \"%2\" %3 %4 %5"}
				.arg(index)
				.arg(QString::fromStdString(model->Bones[attachment->Bone->ArrayIndex]->Name))
				.arg(attachment->Origin.x, 0, 'f', 2)
				.arg(attachment->Origin.y, 0, 'f', 2)
				.arg(attachment->Origin.z, 0, 'f', 2);
		});

	initializePageWithList("Sequences", {}, model->Sequences,
		[&](int index, const auto& sequence)
		{
			const auto name = QString::fromStdString(sequence->Label);
			QString result = QString{"$sequence "} + name;

			// Append a sequence name for each blend. We can't use actual filenames since we don't know them here.
			for (std::size_t blend = 0; blend < sequence->AnimationBlends.size(); ++blend)
			{
				result += QString{" \"%1_%2\""}.arg(name).arg(blend + 1);
			}

			result += " fps " + QString::number(sequence->FPS);

			if (sequence->Flags & STUDIO_LOOPING)
			{
				result += " loop";
			}

			if (sequence->BlendData[0].Type != 0)
			{
				result += QString{" blend %1 %2 %3"}
					.arg(ControlToString(sequence->BlendData[0].Type))
					.arg(sequence->BlendData[0].Start)
					.arg(sequence->BlendData[0].End);
			}

			const auto activityName = (sequence->Activity >= 0 && sequence->Activity <= ACT_FLINCH_RIGHTLEG)
				? activity_map[sequence->Activity].name
				: "ACT_UNKNOWN";
			result += QString{" "} + activityName;
			result += " " + QString::number(sequence->ActivityWeight);

			if (sequence->MotionType != 0)
			{
				result += QString{" "} + ControlToString(sequence->MotionType);
			}

			if (sequence->EntryNode != 0)
			{
				if (sequence->EntryNode == sequence->ExitNode)
				{
					result += QString{" node %1"}.arg(sequence->EntryNode);
				}
				else
				{
					if (sequence->NodeFlags == 0)
					{
						result += QString{" transition %1 %2"}.arg(sequence->EntryNode).arg(sequence->ExitNode);
					}
					else
					{
						result += QString{" rtransition %1 %2"}.arg(sequence->EntryNode).arg(sequence->ExitNode);
					}
				}
			}

			if (!sequence->Events.empty())
			{
				result += " {";

				for (const auto& event : sequence->Events)
				{
					const QString options = event->Options.empty()
						? QString {}
						: QString{" \"%1\""}.arg(QString::fromStdString(event->Options));

					result += QString{"\n\t{ event %1 %2%3 }"}.arg(event->EventId).arg(event->Frame).arg(options);
				}

				result += "\n}";
			}

			return result;
		});

	initializePageWithList("Sequence Groups",
		"Asset Manager merges sequence groups on load so this will always show only the <b>default</b> group",
		model->SequenceGroups,
		[&](int index, const auto& sequenceGroup)
		{
			return QString{"$sequencegroup %1"}.arg(QString::fromStdString(sequenceGroup->Label));
		});

	initializePageWithTextEdit("Textures", R"(Only texture render modes are specified in the QC file<br/>
The textures are referenced in the smds specified in <b>$body</b> and <b>$bodygroup</b> commands<br/>
Note that only <b><u>additive</u></b> and <b><u>masked</u></b> are supported by original compilers, the others are added by custom compilers<br/>
The compiler will automatically mark textures whose name contains the text <b>"chrome"</b> to use the <b>chrome</b> and <b>flatshade</b> render modes)",
		model->Textures,
		[&](QPlainTextEdit* textEdit)
		{
			for (const auto& texture : model->Textures)
			{
				textEdit->appendPlainText(QString{"//\"%1\" (%2)"}
					.arg(QString::fromStdString(texture->Name))
					.arg(texture->ArrayIndex));

				// Generate a command for each render mode flag that is set.
				for (const int renderMode :
				{
					STUDIO_NF_ADDITIVE,
					STUDIO_NF_MASKED,
					STUDIO_NF_FULLBRIGHT,
					STUDIO_NF_CHROME,
					STUDIO_NF_FLATSHADE
				})
				{
					if (texture->Flags & renderMode)
					{
						auto command = QString{"$texrendermode \"%1\" %2"}.arg(QString::fromStdString(texture->Name));

						switch (renderMode)
						{
						case STUDIO_NF_ADDITIVE:
							command = command.arg("additive");
							break;

						case STUDIO_NF_MASKED:
							command = command.arg("masked");
							break;

						case STUDIO_NF_FULLBRIGHT:
							command = command.arg("fullbright");
							break;

						case STUDIO_NF_CHROME:
							command = command.arg("chrome");
							break;

						case STUDIO_NF_FLATSHADE:
							command = command.arg("flatshade");
							break;
						}

						textEdit->appendPlainText(command);
					}
				}
			}
		});

	initializePageWithTextEdit("Skin Families", {}, model->SkinFamilies,
		[&](QPlainTextEdit* textEdit)
		{
			textEdit->setPlaceholderText("// No need for $texturegroup when there is only one skin family");

			if (model->SkinFamilies.size() > 1)
			{
				QString result = QString{"$texturegroup group1\n{"};

				for (std::size_t index = 0; index < model->SkinFamilies[0].size(); ++index)
				{
					// Only include them if at least two unique textures are used for this index.
					if (std::find_if(model->SkinFamilies.begin() + 1, model->SkinFamilies.end(),
						[&](const auto& candidate)
						{
							return model->SkinFamilies[0][index]->Name != candidate[index]->Name;
						}) == model->SkinFamilies.end())
					{
						continue;
					}

					result += "\n{";

					for (bool first = true; const auto& skinFamily : model->SkinFamilies)
					{
						if (first)
						{
							first = false;
						}
						else
						{
							result += " ";
						}

						result += QString{"\"%1\""}.arg(QString::fromStdString(skinFamily[index]->Name));
					}

					result += "}";
				}

				result += "\n}";

				textEdit->appendPlainText(result);
			}
		});

	initializePage("Transitions",
		"Transitions are generated automatically by the compiler when <b>node</b>, <b>transition</b> or <b>rtransition</b> options are encountered in an $sequence command",
		model->Transitions, false);
}

QCDataDialog::~QCDataDialog() = default;
}
